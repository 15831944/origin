#include "LeetCode_function.h"







//给定一个链表，删除链表的倒数第 n 个节点，并且返回链表的头结点
struct ListNode* removeNthFromEnd(struct ListNode* head, int n) {
    struct ListNode *p = head;
    struct ListNode *q = head;
    struct ListNode *r = head;
    int i = 0;

    for(i = 0; i<n; i++){
        p = p->next;
    }

    if(p == NULL){
        r = head;
        head = head->next;
        if(r) free(r);
        return head;
    }

    while(p->next){
        p = p->next;
        q = q->next;
    }

    r = q->next;
    q->next = q->next->next;
    if(r) free(r);
    return head;

}

//反转一个单链表。
struct ListNode* reverseList(struct ListNode* head) {
    return head;
}

//获取文件名
char *GetLastStr(char *pszstr)
{
    return strrchr(pszstr,'\\');
}

struct ListNode* mergeTwoLists2(struct ListNode* l1, struct ListNode* l2) 
{
    if(!l1) return l2;
    if(!l2) return l1;
    struct ListNode* a = l1;
    struct ListNode* b = l2;
    struct ListNode* temp1;
    struct ListNode* temp2;

    while(true){
        temp1 = a->next;
        temp2 = b->next;
        a->next = b;
        if(!temp1) break;
        a = b->next = temp1;     
        if(!temp2) break;
        b = temp2;
    }
    return l1;
}

struct ListNode* mergeTwoLists(struct ListNode* l1, struct ListNode* l2) {
    if(l1==NULL)
        return l2;
    if(l2==NULL)
        return l1;
    if(l1->val < l2->val){
        l1->next = mergeTwoLists(l1->next,l2);
        return l1;
    }else{
        l2->next = mergeTwoLists(l1,l2->next);
        return l2;
    }
}

int Sum(struct ListNode* l2) {
    int sum = 0;
    if(l2==NULL)
        return 0;

    sum += Sum(l2); 
    return sum;

}