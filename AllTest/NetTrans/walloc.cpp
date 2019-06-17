/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

/** 
 * File:        librsync/walloc.cpp
 * Description: 
 */

/*
 * Resource allocation code... the code here is responsible for making
 * sure that nothing leaks.
 *
 * rst --- 4/95 --- 6/95
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef OS2
#define INCL_DOS
#include <os2.h>
#endif

#include "nspr.h"
#include "wconfig.h"
#include "backup_proto.h"
#include "wsnprintf.h"
#include "walloc.h"
#include "util.h"

/* debugging support, define this to enable code which helps detect re-use
 * of freed memory and other such nonsense.
 *
 * The theory is simple.  The FILL_BYTE (0xa5) is written over all malloc'd
 * memory as we receive it, and is written over everything that we free up
 * during a clear_pool.  We check that blocks on the free list always
 * have the FILL_BYTE in them, and we check during palloc() that the bytes
 * still have FILL_BYTE in them.  If you ever see garbage URLs or whatnot
 * containing lots of 0xa5s then you know something used data that's been
 * freed or uninitialized.
 */
/* #define ALLOC_DEBUG */

/* debugging support, if defined all allocations will be done with
 * malloc and free()d appropriately at the end.  This is intended to be
 * used with something like Electric Fence or Purify to help detect
 * memory problems.  Note that if you're using efence then you should also
 * add in ALLOC_DEBUG.  But don't add in ALLOC_DEBUG if you're using Purify
 * because ALLOC_DEBUG would hide all the uninitialized read errors that
 * Purify can diagnose.
 */
/* #define ALLOC_USE_MALLOC */

/* Pool debugging support.  This is intended to detect cases where the
 * wrong pool is used when assigning data to an object in another pool.
 * In particular, it causes the table_{set,add,merge}n routines to check
 * that their arguments are safe for the table they're being placed in.
 * It currently only works with the unix multiprocess model, but could
 * be extended to others.
 */
/* #define POOL_DEBUG */

/* Provide diagnostic information about make_table() calls which are
 * possibly too small.  This requires a recent gcc which supports
 * __builtin_return_address().  The error_log output will be a
 * message such as:
 *    table_push: table created by 0x804d874 hit limit of 10
 * Use "l *0x804d874" to find the source that corresponds to.  It
 * indicates that a table allocated by a call at that address has
 * possibly too small an initial table size guess.
 */
/* #define MAKE_TABLE_PROFILE */

/* Provide some statistics on the cost of allocations.  It requires a
 * bit of an understanding of how alloc.c works.
 */
/* #define ALLOC_STATS */

#ifdef POOL_DEBUG
#ifdef ALLOC_USE_MALLOC
# error "sorry, no support for ALLOC_USE_MALLOC and POOL_DEBUG at the same time"
#endif
#ifdef MULTITHREAD
# error "sorry, no support for MULTITHREAD and POOL_DEBUG at the same time"
#endif
#endif

#ifdef ALLOC_USE_MALLOC
#undef BLOCK_MINFREE
#undef BLOCK_MINALLOC
#define BLOCK_MINFREE   0
#define BLOCK_MINALLOC  0
#endif

int ap_acquire_mutex(PRLock *m)
{
    PR_Lock((PRLock *)m);
    return WAVETOP_BACKUP_OK;
}

int ap_release_mutex(PRLock *m)
{
    PR_Unlock((PRLock *)m);
    return WAVETOP_BACKUP_OK;
}

PRLock *ap_create_mutex(const char *name)
{
    return PR_NewLock();
}

void ap_destroy_mutex(PRLock *m)
{
    PR_DestroyLock((PRLock *)m);
}




/*****************************************************************
 *
 * Managing free storage blocks...
 */

union align {
    /* Types which are likely to have the longest RELEVANT alignment
     * restrictions...
     */

    char *cp;
    void (*f) (void);
    long l;
    FILE *fp;
    double d;
};

#define CLICK_SZ (sizeof(union align))

union block_hdr {
    union align a;

    /* Actual header... */

    struct {
    char *endp;
    union block_hdr *next;
    char *first_avail;
#ifdef POOL_DEBUG
    union block_hdr *global_next;
    struct pool *owning_pool;
#endif
    } h;
};

static union block_hdr *block_freelist = NULL;
static PRLock *alloc_mutex = NULL;
static PRLock *spawn_mutex = NULL;
#ifdef POOL_DEBUG
static char *known_stack_point;
static int stack_direction;
static union block_hdr *global_block_list;
#define FREE_POOL   ((struct pool *)(-1))
#endif
#ifdef ALLOC_STATS
static unsigned long long num_free_blocks_calls;
static unsigned long long num_blocks_freed;
static unsigned max_blocks_in_one_free;
static unsigned num_malloc_calls;
static unsigned num_malloc_bytes;
#endif

#ifdef ALLOC_DEBUG
#define FILL_BYTE   ((char)(0xa5))

#define debug_fill(ptr,size)    ((void)memset((ptr), FILL_BYTE, (size)))

static ap_inline void debug_verify_filled(const char *ptr,
    const char *endp, const char *error_msg)
{
    for (; ptr < endp; ++ptr) {
    if (*ptr != FILL_BYTE) {
        fputs(error_msg, stderr);
        abort();
        exit(1);
    }
    }
}

#else
#define debug_fill(a,b)
#define debug_verify_filled(a,b,c)
#endif


/* Get a completely new block from the system pool. Note that we rely on
   malloc() to provide aligned memory. */

static union block_hdr *malloc_block(int size)
{
    union block_hdr *blok;
    int request_size;

#ifdef ALLOC_DEBUG
    /* make some room at the end which we'll fill and expect to be
     * always filled
     */
    size += CLICK_SZ;
#endif
#ifdef ALLOC_STATS
    ++num_malloc_calls;
    num_malloc_bytes += size + sizeof(union block_hdr);
#endif
    request_size = size + sizeof(union block_hdr);
    blok = (union block_hdr *) malloc(request_size);
    if (blok == NULL) {
    fprintf(stderr, "Ouch!  malloc(%d) failed in malloc_block()\n",
                request_size);
    exit(1);
    }

    debug_fill(blok, size + sizeof(union block_hdr));
    blok->h.next = NULL;
    blok->h.first_avail = (char *) (blok + 1);
    blok->h.endp = size + blok->h.first_avail;
#ifdef ALLOC_DEBUG
    blok->h.endp -= CLICK_SZ;
#endif
#ifdef POOL_DEBUG
    blok->h.global_next = global_block_list;
    global_block_list = blok;
    blok->h.owning_pool = NULL;
#endif

    return blok;
}



#if defined(ALLOC_DEBUG) && !defined(ALLOC_USE_MALLOC)
static void chk_on_blk_list(union block_hdr *blok, union block_hdr *free_blk)
{
    debug_verify_filled(blok->h.endp, blok->h.endp + CLICK_SZ,
    "Ouch!  Someone trounced the padding at the end of a block!\n");
    while (free_blk) {
    if (free_blk == blok) {
        fprintf(stderr, "Ouch!  Freeing free block\n");
        abort();
        exit(1);
    }
    free_blk = free_blk->h.next;
    }
}
#else
#define chk_on_blk_list(_x, _y)
#endif

/* Free a chain of blocks --- must be called with alarms blocked. */

static void free_blocks(union block_hdr *blok)
{
#ifdef ALLOC_USE_MALLOC
    union block_hdr *next;

    for (; blok; blok = next) {
    next = blok->h.next;
    free(blok);
    }
#else
#ifdef ALLOC_STATS
    unsigned num_blocks;
#endif
    /* First, put new blocks at the head of the free list ---
     * we'll eventually bash the 'next' pointer of the last block
     * in the chain to point to the free blocks we already had.
     */

    union block_hdr *old_free_list;

    if (blok == NULL)
    return;         /* Sanity check --- freeing empty pool? */

    (void) ap_acquire_mutex(alloc_mutex);
    old_free_list = block_freelist;
    block_freelist = blok;

    /*
     * Next, adjust first_avail pointers of each block --- have to do it
     * sooner or later, and it simplifies the search in new_block to do it
     * now.
     */

#ifdef ALLOC_STATS
    num_blocks = 1;
#endif
    while (blok->h.next != NULL) {
#ifdef ALLOC_STATS
    ++num_blocks;
#endif
    chk_on_blk_list(blok, old_free_list);
    blok->h.first_avail = (char *) (blok + 1);
    debug_fill(blok->h.first_avail, blok->h.endp - blok->h.first_avail);
#ifdef POOL_DEBUG
    blok->h.owning_pool = FREE_POOL;
#endif
    blok = blok->h.next;
    }

    chk_on_blk_list(blok, old_free_list);
    blok->h.first_avail = (char *) (blok + 1);
    debug_fill(blok->h.first_avail, blok->h.endp - blok->h.first_avail);
#ifdef POOL_DEBUG
    blok->h.owning_pool = FREE_POOL;
#endif

    /* Finally, reset next pointer to get the old free blocks back */

    blok->h.next = old_free_list;

#ifdef ALLOC_STATS
    if (num_blocks > max_blocks_in_one_free) {
    max_blocks_in_one_free = num_blocks;
    }
    ++num_free_blocks_calls;
    num_blocks_freed += num_blocks;
#endif

    (void) ap_release_mutex(alloc_mutex);
#endif
}

/* Really free the memory into the system-pool */
static void free_blocks2(union block_hdr *blok)
{
    union block_hdr *next;

    for (; blok; blok = next) {
        next = blok->h.next;
        free(blok);
    }
}

/* Get a new block, from our own free list if possible, from the system
 * if necessary.  Must be called with alarms blocked.
 */

static union block_hdr *new_block(int min_size)
{
    union block_hdr **lastptr = &block_freelist;
    union block_hdr *blok = block_freelist;

    /* First, see if we have anything of the required size
     * on the free list...
     */

    while (blok != NULL) {
    if (min_size + BLOCK_MINFREE <= blok->h.endp - blok->h.first_avail) {
        *lastptr = blok->h.next;
        blok->h.next = NULL;
        debug_verify_filled(blok->h.first_avail, blok->h.endp,
        "Ouch!  Someone trounced a block on the free list!\n");
        return blok;
    }
    else {
        lastptr = &blok->h.next;
        blok = blok->h.next;
    }
    }

    /* Nope. */

    min_size += BLOCK_MINFREE;
    blok = malloc_block((min_size > BLOCK_MINALLOC) ? min_size : BLOCK_MINALLOC);
    return blok;
}


/* Accounting */

static long bytes_in_block_list(union block_hdr *blok)
{
    long size = 0;

    while (blok) {
    size += blok->h.endp - (char *) (blok + 1);
    blok = blok->h.next;
    }

    return size;
}


/*****************************************************************
 *
 * Pool internals and management...
 * NB that subprocesses are not handled by the generic cleanup code,
 * basically because we don't want cleanups for multiple subprocesses
 * to result in multiple three-second pauses.
 */

struct cleanup;


struct pool {
    union block_hdr *first;
    union block_hdr *last;
    struct cleanup *cleanups;
    struct process_chain *subprocesses;
    struct pool *sub_pools;
    struct pool *sub_next;
    struct pool *sub_prev;
    struct pool *parent;
    char *free_first_avail;
#ifdef ALLOC_USE_MALLOC
    void *allocation_list;
#endif
#ifdef POOL_DEBUG
    struct pool *joined;
#endif
};

static pool *permanent_pool;

/* Each pool structure is allocated in the start of its own first block,
 * so we need to know how many bytes that is (once properly aligned...).
 * This also means that when a pool's sub-pool is destroyed, the storage
 * associated with it is *completely* gone, so we have to make sure it
 * gets taken off the parent's sub-pool list...
 */

#define POOL_HDR_CLICKS (1 + ((sizeof(struct pool) - 1) / CLICK_SZ))
#define POOL_HDR_BYTES (POOL_HDR_CLICKS * CLICK_SZ)

API_EXPORT(pool *) ap_make_sub_pool(struct pool *p)
{
    union block_hdr *blok;
    pool *new_pool;


    (void) ap_acquire_mutex(alloc_mutex);

    blok = new_block(POOL_HDR_BYTES);
    new_pool = (pool *) blok->h.first_avail;
    blok->h.first_avail += POOL_HDR_BYTES;
#ifdef POOL_DEBUG
    blok->h.owning_pool = new_pool;
#endif

    memset((char *) new_pool, '\0', sizeof(struct pool));
    new_pool->free_first_avail = blok->h.first_avail;
    new_pool->first = new_pool->last = blok;

    if (p) {
    new_pool->parent = p;
    new_pool->sub_next = p->sub_pools;
    if (new_pool->sub_next)
        new_pool->sub_next->sub_prev = new_pool;
    p->sub_pools = new_pool;
    }

    (void) ap_release_mutex(alloc_mutex);

    return new_pool;
}

#ifdef POOL_DEBUG
static void stack_var_init(char *s)
{
    char t;

    if (s < &t) {
    stack_direction = 1; /* stack grows up */
    }
    else {
    stack_direction = -1; /* stack grows down */
    }
}
#endif

#ifdef ALLOC_STATS
static void dump_stats(void)
{
    fprintf(stderr,
    "alloc_stats: [%d] #free_blocks %llu #blocks %llu max %u #malloc %u #bytes %u\n",
    (int)getpid(),
    num_free_blocks_calls,
    num_blocks_freed,
    max_blocks_in_one_free,
    num_malloc_calls,
    num_malloc_bytes);
}
#endif

API_EXPORT(pool *) ap_init_alloc(void)
{
#ifdef POOL_DEBUG
    char s;

    known_stack_point = &s;
    stack_var_init(&s);
#endif
    alloc_mutex = ap_create_mutex(NULL);
    spawn_mutex = ap_create_mutex(NULL);
    permanent_pool = ap_make_sub_pool(NULL);
#ifdef ALLOC_STATS
    atexit(dump_stats);
#endif

    return permanent_pool;
}

API_EXPORT(void) ap_cleanup_alloc(void)
{
    ap_destroy_mutex(alloc_mutex);
    ap_destroy_mutex(spawn_mutex);
}

API_EXPORT(void) ap_clear_pool(struct pool *a)
{
    (void) ap_acquire_mutex(alloc_mutex);
    while (a->sub_pools)
    ap_destroy_pool(a->sub_pools);
    (void) ap_release_mutex(alloc_mutex);
    /* Don't hold the mutex during cleanups. */
    /* run_cleanups(a->cleanups); */
    a->cleanups = NULL;
    a->subprocesses = NULL;
    free_blocks(a->first->h.next);
    a->first->h.next = NULL;

    a->last = a->first;
    a->first->h.first_avail = a->free_first_avail;
    debug_fill(a->first->h.first_avail,
    a->first->h.endp - a->first->h.first_avail);

#ifdef ALLOC_USE_MALLOC
    {
    void *c, *n;

    for (c = a->allocation_list; c; c = n) {
        n = *(void **)c;
        free(c);
    }
    a->allocation_list = NULL;
    }
#endif

}

/* Really free the memory into the system-pool */
API_EXPORT(void) ap_clear_pool2(struct pool *a)
{
    (void) ap_acquire_mutex(alloc_mutex);
    while (a->sub_pools)
    ap_destroy_pool(a->sub_pools);
    (void) ap_release_mutex(alloc_mutex);
    /* Don't hold the mutex during cleanups. */
    /* run_cleanups(a->cleanups); */
    a->cleanups = NULL;
    a->subprocesses = NULL;
    free_blocks2(a->first->h.next);
    a->first->h.next = NULL;

    a->last = a->first;
    a->first->h.first_avail = a->free_first_avail;
    debug_fill(a->first->h.first_avail,
    a->first->h.endp - a->first->h.first_avail);

#ifdef ALLOC_USE_MALLOC
    {
    void *c, *n;

    for (c = a->allocation_list; c; c = n) {
        n = *(void **)c;
        free(c);
    }
    a->allocation_list = NULL;
    }
#endif

}

API_EXPORT(void) ap_destroy_pool(pool *a)
{
    ap_clear_pool(a);

    (void) ap_acquire_mutex(alloc_mutex);
    if (a->parent) {
    if (a->parent->sub_pools == a)
        a->parent->sub_pools = a->sub_next;
    if (a->sub_prev)
        a->sub_prev->sub_next = a->sub_next;
    if (a->sub_next)
        a->sub_next->sub_prev = a->sub_prev;
    }
    (void) ap_release_mutex(alloc_mutex);

    free_blocks(a->first);
}

/* Really free the memory into the system-pool */
API_EXPORT(void) ap_destroy_pool2(pool *a)
{
    ap_clear_pool2(a);

    (void) ap_acquire_mutex(alloc_mutex);
    if (a->parent) {
    if (a->parent->sub_pools == a)
        a->parent->sub_pools = a->sub_next;
    if (a->sub_prev)
        a->sub_prev->sub_next = a->sub_next;
    if (a->sub_next)
        a->sub_next->sub_prev = a->sub_prev;
    }
    (void) ap_release_mutex(alloc_mutex);

    free_blocks2(a->first);
}

long ap_bytes_in_pool(pool *p)
{
    return bytes_in_block_list(p->first);
}

long ap_bytes_in_free_blocks(void)
{
    return bytes_in_block_list(block_freelist);
}

/*****************************************************************
 * POOL_DEBUG support
 */
#ifdef POOL_DEBUG

/* the unix linker defines this symbol as the last byte + 1 of
 * the executable... so it includes TEXT, BSS, and DATA
 */
extern char _end;

/* is ptr in the range [lo,hi) */
#define is_ptr_in_range(ptr, lo, hi)    \
    (((unsigned long)(ptr) - (unsigned long)(lo)) \
    < \
    (unsigned long)(hi) - (unsigned long)(lo))

/* Find the pool that ts belongs to, return NULL if it doesn't
 * belong to any pool.
 */
API_EXPORT(pool *) ap_find_pool(const void *ts)
{
    const char *s = ts;
    union block_hdr **pb;
    union block_hdr *b;

    /* short-circuit stuff which is in TEXT, BSS, or DATA */
    if (is_ptr_in_range(s, 0, &_end)) {
    return NULL;
    }
    /* consider stuff on the stack to also be in the NULL pool...
     * XXX: there's cases where we don't want to assume this
     */
    if ((stack_direction == -1 && is_ptr_in_range(s, &ts, known_stack_point))
    || (stack_direction == 1 && is_ptr_in_range(s, known_stack_point, &ts))) {
    abort();
    return NULL;
    }
    /* ap_block_alarms(); */
    /* search the global_block_list */
    for (pb = &global_block_list; *pb; pb = &b->h.global_next) {
    b = *pb;
    if (is_ptr_in_range(s, b, b->h.endp)) {
        if (b->h.owning_pool == FREE_POOL) {
        fprintf(stderr,
            "Ouch!  find_pool() called on pointer in a free block\n");
        abort();
        exit(1);
        }
        if (b != global_block_list) {
        /* promote b to front of list, this is a hack to speed
         * up the lookup */
        *pb = b->h.global_next;
        b->h.global_next = global_block_list;
        global_block_list = b;
        }
        /* ap_unblock_alarms(); */
        return b->h.owning_pool;
    }
    }
    /* ap_unblock_alarms(); */
    return NULL;
}

/* return TRUE iff a is an ancestor of b
 * NULL is considered an ancestor of all pools
 */
API_EXPORT(int) ap_pool_is_ancestor(pool *a, pool *b)
{
    if (a == NULL) {
    return 1;
    }
    while (a->joined) {
    a = a->joined;
    }
    while (b) {
    if (a == b) {
        return 1;
    }
    b = b->parent;
    }
    return 0;
}

/* All blocks belonging to sub will be changed to point to p
 * instead.  This is a guarantee by the caller that sub will not
 * be destroyed before p is.
 */
API_EXPORT(void) ap_pool_join(pool *p, pool *sub)
{
    union block_hdr *b;

    /* We could handle more general cases... but this is it for now. */
    if (sub->parent != p) {
    fprintf(stderr, "pool_join: p is not parent of sub\n");
    abort();
    }
    /* ap_block_alarms(); */
    while (p->joined) {
    p = p->joined;
    }
    sub->joined = p;
    for (b = global_block_list; b; b = b->h.global_next) {
    if (b->h.owning_pool == sub) {
        b->h.owning_pool = p;
    }
    }
    /* ap_unblock_alarms(); */
}
#endif

/*****************************************************************
 *
 * Allocating stuff...
 */


API_EXPORT(void *) ap_palloc(struct pool *a, int reqsize)
{
#ifdef ALLOC_USE_MALLOC
    int size = reqsize + CLICK_SZ;
    void *ptr;

    /* ap_block_alarms(); */
    ptr = malloc(size);
    if (ptr == NULL) {
    fputs("Ouch!  Out of memory!\n", stderr);
    exit(1);
    }
    debug_fill(ptr, size); /* might as well get uninitialized protection */
    *(void **)ptr = a->allocation_list;
    a->allocation_list = ptr;
    /* ap_unblock_alarms(); */ 
    return (char *)ptr + CLICK_SZ;
#else

    /* Round up requested size to an even number of alignment units (core clicks)
     */

    int nclicks = 1 + ((reqsize - 1) / CLICK_SZ);
    int size = nclicks * CLICK_SZ;

    /* First, see if we have space in the block most recently
     * allocated to this pool
     */

    union block_hdr *blok = a->last;
    char *first_avail = blok->h.first_avail;
    char *new_first_avail;

    if (reqsize <= 0)
    return NULL;

    new_first_avail = first_avail + size;

    if (new_first_avail <= blok->h.endp) {
    debug_verify_filled(first_avail, blok->h.endp,
        "Ouch!  Someone trounced past the end of their allocation!\n");
    blok->h.first_avail = new_first_avail;
    return (void *) first_avail;
    }

    /* Nope --- get a new one that's guaranteed to be big enough */

    /* ap_block_alarms(); */

    (void) ap_acquire_mutex(alloc_mutex);

    blok = new_block(size);
    a->last->h.next = blok;
    a->last = blok;
#ifdef POOL_DEBUG
    blok->h.owning_pool = a;
#endif

    (void) ap_release_mutex(alloc_mutex);

    /* ap_unblock_alarms(); */

    first_avail = blok->h.first_avail;
    blok->h.first_avail += size;

    return (void *) first_avail;
#endif
}

API_EXPORT(void *) ap_pcalloc(struct pool *a, int size)
{
    void *res = ap_palloc(a, size);
    memset(res, '\0', size);
    return res;
}

API_EXPORT(char *) ap_pstrdup(struct pool *a, const char *s)
{
    char *res;
    size_t len;

    if (s == NULL)
    return NULL;
    len = strlen(s) + 1;
    res = (char *)ap_palloc(a, len);
    memcpy(res, s, len);
    return res;
}

API_EXPORT(char *) ap_pstrndup(struct pool *a, const char *s, int n)
{
    char *res;

    if (s == NULL)
    return NULL;
    res = (char *)ap_palloc(a, n + 1);
    memcpy(res, s, n);
    res[n] = '\0';
    return res;
}

API_EXPORT(char *) ap_pstrcat(pool *a,...)
{
    char *cp, *argp, *res;

    /* Pass one --- find length of required string */

    int len = 0;
    va_list adummy;

    va_start(adummy, a);

    while ((cp = va_arg(adummy, char *)) != NULL)
         len += strlen(cp);

    va_end(adummy);

    /* Allocate the required string */

    res = (char *) ap_palloc(a, len + 1);
    cp = res;
    *cp = '\0';

    /* Pass two --- copy the argument strings into the result space */

    va_start(adummy, a);

    while ((argp = va_arg(adummy, char *)) != NULL) {
    strcpy(cp, argp);
    cp += strlen(argp);
    }

    va_end(adummy);

    /* Return the result string */

    return res;
}

/* ap_psprintf is implemented by writing directly into the current
 * block of the pool, starting right at first_avail.  If there's
 * insufficient room, then a new block is allocated and the earlier
 * output is copied over.  The new block isn't linked into the pool
 * until all the output is done.
 *
 * Note that this is completely safe because nothing else can
 * allocate in this pool while ap_psprintf is running.  alarms are
 * blocked, and the only thing outside of alloc.c that's invoked
 * is ap_vformatter -- which was purposefully written to be
 * self-contained with no callouts.
 */

struct psprintf_data {
    ap_vformatter_buff vbuff;
#ifdef ALLOC_USE_MALLOC
    char *base;
#else
    union block_hdr *blok;
    int got_a_new_block;
#endif
};

#define AP_PSPRINTF_MIN_SIZE 32  /* Minimum size of allowable avail block */

static int psprintf_flush(ap_vformatter_buff *vbuff)
{
    struct psprintf_data *ps = (struct psprintf_data *)vbuff;
#ifdef ALLOC_USE_MALLOC
    int cur_len, size;
    char *ptr;

    cur_len = (char *)ps->vbuff.curpos - ps->base;
    size = cur_len << 1;
    if (size < AP_PSPRINTF_MIN_SIZE)
        size = AP_PSPRINTF_MIN_SIZE;
    ptr = realloc(ps->base, size);
    if (ptr == NULL) {
    fputs("Ouch!  Out of memory!\n", stderr);
    exit(1);
    }
    ps->base = ptr;
    ps->vbuff.curpos = ptr + cur_len;
    ps->vbuff.endpos = ptr + size - 1;
    return 0;
#else
    union block_hdr *blok;
    union block_hdr *nblok;
    size_t cur_len, size;
    char *strp;

    blok = ps->blok;
    strp = ps->vbuff.curpos;
    cur_len = strp - blok->h.first_avail;
    size = cur_len << 1;
    if (size < AP_PSPRINTF_MIN_SIZE)
        size = AP_PSPRINTF_MIN_SIZE;

    /* must try another blok */
    (void) ap_acquire_mutex(alloc_mutex);
    nblok = new_block(size);
    (void) ap_release_mutex(alloc_mutex);
    memcpy(nblok->h.first_avail, blok->h.first_avail, cur_len);
    ps->vbuff.curpos = nblok->h.first_avail + cur_len;
    /* save a byte for the NUL terminator */
    ps->vbuff.endpos = nblok->h.endp - 1;

    /* did we allocate the current blok? if so free it up */
    if (ps->got_a_new_block) {
    debug_fill(blok->h.first_avail, blok->h.endp - blok->h.first_avail);
    (void) ap_acquire_mutex(alloc_mutex);
    blok->h.next = block_freelist;
    block_freelist = blok;
    (void) ap_release_mutex(alloc_mutex);
    }
    ps->blok = nblok;
    ps->got_a_new_block = 1;
    /* note that we've deliberately not linked the new block onto
     * the pool yet... because we may need to flush again later, and
     * we'd have to spend more effort trying to unlink the block.
     */
    return 0;
#endif
}

API_EXPORT(char *) ap_pvsprintf(pool *p, const char *fmt, va_list ap)
{
#ifdef ALLOC_USE_MALLOC
    struct psprintf_data ps;
    void *ptr;

    /* ap_block_alarms(); */
    ps.base = malloc(512);
    if (ps.base == NULL) {
    fputs("Ouch!  Out of memory!\n", stderr);
    exit(1);
    }
    /* need room at beginning for allocation_list */
    ps.vbuff.curpos = ps.base + CLICK_SZ;
    ps.vbuff.endpos = ps.base + 511;
    ap_vformatter(psprintf_flush, &ps.vbuff, fmt, ap);
    *ps.vbuff.curpos++ = '\0';
    ptr = ps.base;
    /* shrink */
    ptr = realloc(ptr, (char *)ps.vbuff.curpos - (char *)ptr);
    if (ptr == NULL) {
    fputs("Ouch!  Out of memory!\n", stderr);
    exit(1);
    }
    *(void **)ptr = p->allocation_list;
    p->allocation_list = ptr;
    /* ap_unblock_alarms(); */
    return (char *)ptr + CLICK_SZ;
#else
    struct psprintf_data ps;
    char *strp;
    int size;

    /* ap_block_alarms(); */
    ps.blok = p->last;
    ps.vbuff.curpos = ps.blok->h.first_avail;
    ps.vbuff.endpos = ps.blok->h.endp - 1;  /* save one for NUL */
    ps.got_a_new_block = 0;

    if (ps.blok->h.first_avail == ps.blok->h.endp)
        psprintf_flush(&ps.vbuff);      /* ensure room for NUL */
    ap_vformatter(psprintf_flush, &ps.vbuff, fmt, ap);

    strp = ps.vbuff.curpos;
    *strp++ = '\0';

    size = strp - ps.blok->h.first_avail;
    size = (1 + ((size - 1) / CLICK_SZ)) * CLICK_SZ;
    strp = ps.blok->h.first_avail;  /* save away result pointer */
    ps.blok->h.first_avail += size;

    /* have to link the block in if it's a new one */
    if (ps.got_a_new_block) {
    p->last->h.next = ps.blok;
    p->last = ps.blok;
#ifdef POOL_DEBUG
    ps.blok->h.owning_pool = p;
#endif
    }
    /* ap_unblock_alarms(); */

    return strp;
#endif
}

API_EXPORT(char *) ap_psprintf(pool *p, const char *fmt, ...)
{
    va_list ap;
    char *res;

    va_start(ap, fmt);
    res = ap_pvsprintf(p, fmt, ap);
    va_end(ap);
    return res;
}

/*****************************************************************
 *
 * The 'array' functions...
 */

static void make_array_core(array_header *res, pool *p, int nelts, int elt_size)
{
    if (nelts < 1)
    nelts = 1;      /* Assure sanity if someone asks for
                 * array of zero elts.
                 */

    res->elts = (char *)ap_pcalloc(p, nelts * elt_size);

    res->pool = p;
    res->elt_size = elt_size;
    res->nelts = 0;     /* No active elements yet... */
    res->nalloc = nelts;    /* ...but this many allocated */
}

API_EXPORT(array_header *) ap_make_array(pool *p, int nelts, int elt_size)
{
    array_header *res = (array_header *) ap_palloc(p, sizeof(array_header));

    make_array_core(res, p, nelts, elt_size);
    return res;
}

API_EXPORT(void *) ap_push_array(array_header *arr)
{
    if (arr->nelts == arr->nalloc) {
    int new_size = (arr->nalloc <= 0) ? 1 : arr->nalloc * 2;
    char *new_data;

    new_data = (char *)ap_pcalloc(arr->pool, arr->elt_size * new_size);

    memcpy(new_data, arr->elts, arr->nalloc * arr->elt_size);
    arr->elts = new_data;
    arr->nalloc = new_size;
    }

    ++arr->nelts;
    return arr->elts + (arr->elt_size * (arr->nelts - 1));
}

API_EXPORT(void) ap_array_cat(array_header *dst, const array_header *src)
{
    int elt_size = dst->elt_size;

    if (dst->nelts + src->nelts > dst->nalloc) {
    int new_size = (dst->nalloc <= 0) ? 1 : dst->nalloc * 2;
    char *new_data;

    while (dst->nelts + src->nelts > new_size)
        new_size *= 2;

    new_data = (char *)ap_pcalloc(dst->pool, elt_size * new_size);
    memcpy(new_data, dst->elts, dst->nalloc * elt_size);

    dst->elts = new_data;
    dst->nalloc = new_size;
    }

    memcpy(dst->elts + dst->nelts * elt_size, src->elts, elt_size * src->nelts);
    dst->nelts += src->nelts;
}

API_EXPORT(array_header *) ap_copy_array(pool *p, const array_header *arr)
{
    array_header *res = ap_make_array(p, arr->nalloc, arr->elt_size);

    memcpy(res->elts, arr->elts, arr->elt_size * arr->nelts);
    res->nelts = arr->nelts;
    return res;
}

/* This cute function copies the array header *only*, but arranges
 * for the data section to be copied on the first push or arraycat.
 * It's useful when the elements of the array being copied are
 * read only, but new stuff *might* get added on the end; we have the
 * overhead of the full copy only where it is really needed.
 */

static ap_inline void copy_array_hdr_core(array_header *res,
    const array_header *arr)
{
    res->elts = arr->elts;
    res->elt_size = arr->elt_size;
    res->nelts = arr->nelts;
    res->nalloc = arr->nelts;   /* Force overflow on push */
}

API_EXPORT(array_header *) ap_copy_array_hdr(pool *p, const array_header *arr)
{
    array_header *res = (array_header *) ap_palloc(p, sizeof(array_header));

    res->pool = p;
    copy_array_hdr_core(res, arr);
    return res;
}

/* The above is used here to avoid consing multiple new array bodies... */

API_EXPORT(array_header *) ap_append_arrays(pool *p,
                     const array_header *first,
                     const array_header *second)
{
    array_header *res = ap_copy_array_hdr(p, first);

    ap_array_cat(res, second);
    return res;
}

/* ap_array_pstrcat generates a new string from the pool containing
 * the concatenated sequence of substrings referenced as elements within
 * the array.  The string will be empty if all substrings are empty or null,
 * or if there are no elements in the array.
 * If sep is non-NUL, it will be inserted between elements as a separator.
 */
API_EXPORT(char *) ap_array_pstrcat(pool *p, const array_header *arr,
                                    const char sep)
{
    char *cp, *res, **strpp;
    int i, len;

    if (arr->nelts <= 0 || arr->elts == NULL)      /* Empty table? */
        return (char *) ap_pcalloc(p, 1);

    /* Pass one --- find length of required string */

    len = 0;
    for (i = 0, strpp = (char **) arr->elts; ; ++strpp) {
        if (strpp && *strpp != NULL) {
            len += strlen(*strpp);
        }
        if (++i >= arr->nelts)
            break;
        if (sep)
            ++len;
    }

    /* Allocate the required string */

    res = (char *) ap_palloc(p, len + 1);
    cp = res;

    /* Pass two --- copy the argument strings into the result space */

    for (i = 0, strpp = (char **) arr->elts; ; ++strpp) {
        if (strpp && *strpp != NULL) {
            len = strlen(*strpp);
            memcpy(cp, *strpp, len);
            cp += len;
        }
        if (++i >= arr->nelts)
            break;
        if (sep)
            *cp++ = sep;
    }

    *cp = '\0';

    /* Return the result string */

    return res;
}


/*****************************************************************
 *
 * The "table" functions.
 */

/* XXX: if you tweak this you should look at is_empty_table() and table_elts()
 * in ap_alloc.h */
struct table {
    /* This has to be first to promote backwards compatibility with
     * older modules which cast a table * to an array_header *...
     * they should use the table_elts() function for most of the
     * cases they do this for.
     */
    array_header a;
#ifdef MAKE_TABLE_PROFILE
    void *creator;
#endif
};

#ifdef MAKE_TABLE_PROFILE
static table_entry *table_push(table *t)
{
    if (t->a.nelts == t->a.nalloc) {
    fprintf(stderr,
        "table_push: table created by %p hit limit of %u\n",
        t->creator, t->a.nalloc);
    }
    return (table_entry *) ap_push_array(&t->a);
}
#else
#define table_push(t)   ((table_entry *) ap_push_array(&(t)->a))
#endif


API_EXPORT(table *) ap_make_table(pool *p, int nelts)
{
    table *t = (table *)ap_palloc(p, sizeof(table));

    make_array_core(&t->a, p, nelts, sizeof(table_entry));
#ifdef MAKE_TABLE_PROFILE
    t->creator = __builtin_return_address(0);
#endif
    return t;
}

API_EXPORT(table *) ap_copy_table(pool *p, const table *t)
{
    table *nnew = (table *)ap_palloc(p, sizeof(table));

#ifdef POOL_DEBUG
    /* we don't copy keys and values, so it's necessary that t->a.pool
     * have a life span at least as long as p
     */
    if (!ap_pool_is_ancestor(t->a.pool, p)) {
    fprintf(stderr, "copy_table: t's pool is not an ancestor of p\n");
    abort();
    }
#endif
    make_array_core(&nnew->a, p, t->a.nalloc, sizeof(table_entry));
    memcpy(nnew->a.elts, t->a.elts, t->a.nelts * sizeof(table_entry));
    nnew->a.nelts = t->a.nelts;
    return nnew;
}

API_EXPORT(void) ap_clear_table(table *t)
{
    t->a.nelts = 0;
}

API_EXPORT(const char *) ap_table_get(const table *t, const char *key)
{
    table_entry *elts = (table_entry *) t->a.elts;
    int i;

    if (key == NULL)
    return NULL;

    for (i = 0; i < t->a.nelts; ++i)
    if (!strcasecmp(elts[i].key, key))
        return elts[i].val;

    return NULL;
}

API_EXPORT(void) ap_table_set(table *t, const char *key, const char *val)
{
    register int i, j, k;
    table_entry *elts = (table_entry *) t->a.elts;
    int done = 0;

    for (i = 0; i < t->a.nelts; ) {
    if (!strcasecmp(elts[i].key, key)) {
        if (!done) {
        elts[i].val = ap_pstrdup(t->a.pool, val);
        done = 1;
        ++i;
        }
        else {      /* delete an extraneous element */
        for (j = i, k = i + 1; k < t->a.nelts; ++j, ++k) {
            elts[j].key = elts[k].key;
            elts[j].val = elts[k].val;
        }
        --t->a.nelts;
        }
    }
    else {
        ++i;
    }
    }

    if (!done) {
    elts = (table_entry *) table_push(t);
    elts->key = ap_pstrdup(t->a.pool, key);
    elts->val = ap_pstrdup(t->a.pool, val);
    }
}

API_EXPORT(void) ap_table_setn(table *t, const char *key, const char *val)
{
    register int i, j, k;
    table_entry *elts = (table_entry *) t->a.elts;
    int done = 0;

#ifdef POOL_DEBUG
    {
    if (!ap_pool_is_ancestor(ap_find_pool(key), t->a.pool)) {
        fprintf(stderr, "table_set: key not in ancestor pool of t\n");
        abort();
    }
    if (!ap_pool_is_ancestor(ap_find_pool(val), t->a.pool)) {
        fprintf(stderr, "table_set: val not in ancestor pool of t\n");
        abort();
    }
    }
#endif

    for (i = 0; i < t->a.nelts; ) {
    if (!strcasecmp(elts[i].key, key)) {
        if (!done) {
        elts[i].val = (char *)val;
        done = 1;
        ++i;
        }
        else {      /* delete an extraneous element */
        for (j = i, k = i + 1; k < t->a.nelts; ++j, ++k) {
            elts[j].key = elts[k].key;
            elts[j].val = elts[k].val;
        }
        --t->a.nelts;
        }
    }
    else {
        ++i;
    }
    }

    if (!done) {
    elts = (table_entry *) table_push(t);
    elts->key = (char *)key;
    elts->val = (char *)val;
    }
}

API_EXPORT(void) ap_table_unset(table *t, const char *key)
{
    register int i, j, k;
    table_entry *elts = (table_entry *) t->a.elts;

    for (i = 0; i < t->a.nelts;) {
    if (!strcasecmp(elts[i].key, key)) {

        /* found an element to skip over
         * there are any number of ways to remove an element from
         * a contiguous block of memory.  I've chosen one that
         * doesn't do a memcpy/bcopy/array_delete, *shrug*...
         */
        for (j = i, k = i + 1; k < t->a.nelts; ++j, ++k) {
        elts[j].key = elts[k].key;
        elts[j].val = elts[k].val;
        }
        --t->a.nelts;
    }
    else {
        ++i;
    }
    }
}

API_EXPORT(void) ap_table_merge(table *t, const char *key, const char *val)
{
    table_entry *elts = (table_entry *) t->a.elts;
    int i;

    for (i = 0; i < t->a.nelts; ++i)
    if (!strcasecmp(elts[i].key, key)) {
        elts[i].val = ap_pstrcat(t->a.pool, elts[i].val, ", ", val, NULL);
        return;
    }

    elts = (table_entry *) table_push(t);
    elts->key = ap_pstrdup(t->a.pool, key);
    elts->val = ap_pstrdup(t->a.pool, val);
}

API_EXPORT(void) ap_table_mergen(table *t, const char *key, const char *val)
{
    table_entry *elts = (table_entry *) t->a.elts;
    int i;

#ifdef POOL_DEBUG
    {
    if (!ap_pool_is_ancestor(ap_find_pool(key), t->a.pool)) {
        fprintf(stderr, "table_set: key not in ancestor pool of t\n");
        abort();
    }
    if (!ap_pool_is_ancestor(ap_find_pool(val), t->a.pool)) {
        fprintf(stderr, "table_set: key not in ancestor pool of t\n");
        abort();
    }
    }
#endif

    for (i = 0; i < t->a.nelts; ++i) {
    if (!strcasecmp(elts[i].key, key)) {
        elts[i].val = ap_pstrcat(t->a.pool, elts[i].val, ", ", val, NULL);
        return;
    }
    }

    elts = (table_entry *) table_push(t);
    elts->key = (char *)key;
    elts->val = (char *)val;
}

API_EXPORT(void) ap_table_add(table *t, const char *key, const char *val)
{
    table_entry *elts = (table_entry *) t->a.elts;

    elts = (table_entry *) table_push(t);
    elts->key = ap_pstrdup(t->a.pool, key);
    elts->val = ap_pstrdup(t->a.pool, val);
}

API_EXPORT(void) ap_table_addn(table *t, const char *key, const char *val)
{
    table_entry *elts = (table_entry *) t->a.elts;

#ifdef POOL_DEBUG
    {
    if (!ap_pool_is_ancestor(ap_find_pool(key), t->a.pool)) {
        fprintf(stderr, "table_set: key not in ancestor pool of t\n");
        abort();
    }
    if (!ap_pool_is_ancestor(ap_find_pool(val), t->a.pool)) {
        fprintf(stderr, "table_set: key not in ancestor pool of t\n");
        abort();
    }
    }
#endif

    elts = (table_entry *) table_push(t);
    elts->key = (char *)key;
    elts->val = (char *)val;
}

API_EXPORT(table *) ap_overlay_tables(pool *p, const table *overlay, const table *base)
{
    table *res;

#ifdef POOL_DEBUG
    /* we don't copy keys and values, so it's necessary that
     * overlay->a.pool and base->a.pool have a life span at least
     * as long as p
     */
    if (!ap_pool_is_ancestor(overlay->a.pool, p)) {
    fprintf(stderr, "overlay_tables: overlay's pool is not an ancestor of p\n");
    abort();
    }
    if (!ap_pool_is_ancestor(base->a.pool, p)) {
    fprintf(stderr, "overlay_tables: base's pool is not an ancestor of p\n");
    abort();
    }
#endif

    res = (table *)ap_palloc(p, sizeof(table));
    /* behave like append_arrays */
    res->a.pool = p;
    copy_array_hdr_core(&res->a, &overlay->a);
    ap_array_cat(&res->a, &base->a);

    return res;
}

/* And now for something completely abstract ...

 * For each key value given as a vararg:
 *   run the function pointed to as
 *     int comp(void *r, char *key, char *value);
 *   on each valid key-value pair in the table t that matches the vararg key,
 *   or once for every valid key-value pair if the vararg list is empty,
 *   until the function returns false (0) or we finish the table.
 *
 * Note that we restart the traversal for each vararg, which means that
 * duplicate varargs will result in multiple executions of the function
 * for each matching key.  Note also that if the vararg list is empty,
 * only one traversal will be made and will cut short if comp returns 0.
 *
 * Note that the table_get and table_merge functions assume that each key in
 * the table is unique (i.e., no multiple entries with the same key).  This
 * function does not make that assumption, since it (unfortunately) isn't
 * true for some of Apache's tables.
 *
 * Note that rec is simply passed-on to the comp function, so that the
 * caller can pass additional info for the task.
 */
API_EXPORT(void) ap_table_do(int (*comp) (void *, const char *, const char *), 
                                void *rec, const table *t,...)
{
    va_list vp;
    char *argp;
    table_entry *elts = (table_entry *) t->a.elts;
    int rv, i;

    va_start(vp, t);

    argp = va_arg(vp, char *);

    do {
    for (rv = 1, i = 0; rv && (i < t->a.nelts); ++i) {
        if (elts[i].key && (!argp || !strcasecmp(elts[i].key, argp))) {
        rv = (*comp) (rec, elts[i].key, elts[i].val);
        }
    }
    } while (argp && ((argp = va_arg(vp, char *)) != NULL));

    va_end(vp);
}

/* Curse libc and the fact that it doesn't guarantee a stable sort.  We
 * have to enforce stability ourselves by using the order field.  If it
 * provided a stable sort then we wouldn't even need temporary storage to
 * do the work below. -djg
 *
 * ("stable sort" means that equal keys retain their original relative
 * ordering in the output.)
 */
typedef struct {
    char *key;
    char *val;
    int order;
} overlap_key;

static int sort_overlap(const void *va, const void *vb)
{
    const overlap_key *a = (const overlap_key *)va;
    const overlap_key *b = (const overlap_key *)vb;
    int r;

    r = strcasecmp(a->key, b->key);
    if (r) {
    return r;
    }
    return a->order - b->order;
}

/* prefer to use the stack for temp storage for overlaps smaller than this */
#ifndef AP_OVERLAP_TABLES_ON_STACK
#define AP_OVERLAP_TABLES_ON_STACK  (512)
#endif

void ap_overlap_tables(table *a, const table *b, unsigned flags)
{
    overlap_key cat_keys_buf[AP_OVERLAP_TABLES_ON_STACK];
    overlap_key *cat_keys;
    int nkeys;
    table_entry *e;
    table_entry *last_e;
    overlap_key *left;
    overlap_key *right;
    overlap_key *last;

    nkeys = a->a.nelts + b->a.nelts;
    if (nkeys < AP_OVERLAP_TABLES_ON_STACK) {
    cat_keys = cat_keys_buf;
    }
    else {
    /* XXX: could use scratch free space in a or b's pool instead...
     * which could save an allocation in b's pool.
     */
    cat_keys = (overlap_key *)ap_palloc(b->a.pool, sizeof(overlap_key) * nkeys);
    }

    nkeys = 0;

    /* Create a list of the entries from a concatenated with the entries
     * from b.
     */
    e = (table_entry *)a->a.elts;
    last_e = e + a->a.nelts;
    while (e < last_e) {
    cat_keys[nkeys].key = e->key;
    cat_keys[nkeys].val = e->val;
    cat_keys[nkeys].order = nkeys;
    ++nkeys;
    ++e;
    }

    e = (table_entry *)b->a.elts;
    last_e = e + b->a.nelts;
    while (e < last_e) {
    cat_keys[nkeys].key = e->key;
    cat_keys[nkeys].val = e->val;
    cat_keys[nkeys].order = nkeys;
    ++nkeys;
    ++e;
    }

    qsort(cat_keys, nkeys, sizeof(overlap_key), sort_overlap);

    /* Now iterate over the sorted list and rebuild a.
     * Start by making sure it has enough space.
     */
    a->a.nelts = 0;
    if (a->a.nalloc < nkeys) {
    a->a.elts = (char *)ap_palloc(a->a.pool, a->a.elt_size * nkeys * 2);
    a->a.nalloc = nkeys * 2;
    }

    /*
     * In both the merge and set cases we retain the invariant:
     *
     * left->key, (left+1)->key, (left+2)->key, ..., (right-1)->key
     * are all equal keys.  (i.e. strcasecmp returns 0)
     *
     * We essentially need to find the maximal
     * right for each key, then we can do a quick merge or set as
     * appropriate.
     */

    if (flags & AP_OVERLAP_TABLES_MERGE) {
    left = cat_keys;
    last = left + nkeys;
    while (left < last) {
        right = left + 1;
        if (right == last
        || strcasecmp(left->key, right->key)) {
        ap_table_addn(a, left->key, left->val);
        left = right;
        }
        else {
        char *strp;
        char *value;
        size_t len;

        /* Have to merge some headers.  Let's re-use the order field,
         * since it's handy... we'll store the length of val there.
         */
        left->order = strlen(left->val);
        len = left->order;
        do {
            right->order = strlen(right->val);
            len += 2 + right->order;
            ++right;
        } while (right < last
            && !strcasecmp(left->key, right->key));
        /* right points one past the last header to merge */
        value = (char *)ap_palloc(a->a.pool, len + 1);
        strp = value;
        for (;;) {
            memcpy(strp, left->val, left->order);
            strp += left->order;
            ++left;
            if (left == right) break;
            *strp++ = ',';
            *strp++ = ' ';
        }
        *strp = 0;
        ap_table_addn(a, (left-1)->key, value);
        }
    }
    }
    else {
    left = cat_keys;
    last = left + nkeys;
    while (left < last) {
        right = left + 1;
        while (right < last && !strcasecmp(left->key, right->key)) {
        ++right;
        }
        ap_table_addn(a, (right-1)->key, (right-1)->val);
        left = right;
    }
    }
}

