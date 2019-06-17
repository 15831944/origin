/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

#ifndef __WAVTOP_SNPRINTF_H_
#define __WAVTOP_SNPRINTF_H_ 1

#include "mirror.h"

/* ap_vformatter() is a generic printf-style formatting routine
 * with some extensions.  The extensions are:
 *
 * %pA	takes a struct in_addr *, and prints it as a.b.c.d
 * %pI	takes a struct sockaddr_in * and prints it as a.b.c.d:port
 * %pp  takes a void * and outputs it in hex
 *
 * The %p hacks are to force gcc's printf warning code to skip
 * over a pointer argument without complaining.  This does
 * mean that the ANSI-style %p (output a void * in hex format) won't
 * work as expected at all, but that seems to be a fair trade-off
 * for the increased robustness of having printf-warnings work.
 *
 * Additionally, ap_vformatter allows for arbitrary output methods
 * using the ap_vformatter_buff and flush_func.
 *
 * The ap_vformatter_buff has two elements curpos and endpos.
 * curpos is where ap_vformatter will write the next byte of output.
 * It proceeds writing output to curpos, and updating curpos, until
 * either the end of output is reached, or curpos == endpos (i.e. the
 * buffer is full).
 *
 * If the end of output is reached, ap_vformatter returns the
 * number of bytes written.
 *
 * When the buffer is full, the flush_func is called.  The flush_func
 * can return -1 to indicate that no further output should be attempted,
 * and ap_vformatter will return immediately with -1.  Otherwise
 * the flush_func should flush the buffer in whatever manner is
 * appropriate, re-initialize curpos and endpos, and return 0.
 *
 * Note that flush_func is only invoked as a result of attempting to
 * write another byte at curpos when curpos >= endpos.  So for
 * example, it's possible when the output exactly matches the buffer
 * space available that curpos == endpos will be true when
 * ap_vformatter returns.
 *
 * ap_vformatter does not call out to any other code, it is entirely
 * self-contained.  This allows the callers to do things which are
 * otherwise "unsafe".  For example, ap_psprintf uses the "scratch"
 * space at the unallocated end of a block, and doesn't actually
 * complete the allocation until ap_vformatter returns.  ap_psprintf
 * would be completely broken if ap_vformatter were to call anything
 * that used a pool.  Similarly http_bprintf() uses the "scratch"
 * space at the end of its output buffer, and doesn't actually note
 * that the space is in use until it either has to flush the buffer
 * or until ap_vformatter returns.
 */

typedef struct {
    char *curpos;
    char *endpos;
} ap_vformatter_buff;

typedef int (*flush_func_cb)(ap_vformatter_buff *);

int ap_vformatter(flush_func_cb flush_func ,
    ap_vformatter_buff *, const char *fmt, va_list ap);

/* These are snprintf implementations based on ap_vformatter().
 *
 * Note that various standards and implementations disagree on the return
 * value of snprintf, and side-effects due to %n in the formatting string.
 * ap_snprintf behaves as follows:
 *
 * Process the format string until the entire string is exhausted, or
 * the buffer fills.  If the buffer fills then stop processing immediately
 * (so no further %n arguments are processed), and return the buffer
 * length.  In all cases the buffer is NUL terminated. The return value
 * is the number of characters placed in the buffer, excluding the
 * terminating NUL. All this implies that, at most, (len-1) characters
 * will be copied over; if the return value is >= len, then truncation
 * occured.
 *
 * In no event does ap_snprintf return a negative number.
 */
int ap_snprintf(char *buf, size_t len, const char *format,...)
			    __attribute__((format(printf,3,4)));
int ap_vsnprintf(char *buf, size_t len, const char *format,
			     va_list ap);

#endif // __WAVTOP_SNPRINTF_H_
