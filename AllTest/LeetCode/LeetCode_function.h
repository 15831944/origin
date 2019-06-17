#ifndef _LEETCODE_FUNCTION_H_
#define _LEETCODE_FUNCTION_H_


#include <iostream>
#include <Windows.h>

using namespace std;

struct ListNode {
         int val;
         struct ListNode *next;
     };
typedef struct ListNode ListNode;



struct ListNode* removeNthFromEnd(struct ListNode* head, int n);

char *GetLastStr(char *pszstr);

struct ListNode* mergeTwoLists(struct ListNode* l1, struct ListNode* l2);























#endif