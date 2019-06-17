#include "LeetCode_function.h"




void mergeTwoLists(){
    ListNode *pst1 = (ListNode *)malloc(sizeof(ListNode));
    ListNode *pst2 = (ListNode *)malloc(sizeof(ListNode));
    ListNode *pst3 = (ListNode *)malloc(sizeof(ListNode));
    ListNode *pst4;

    pst1->val = 11;
    pst1->next = pst2;
    pst2->val = 54;
    pst2->next = pst3;
    pst3->val = 90;
    pst3->next = NULL;

    ListNode *pst11 = (ListNode *)malloc(sizeof(ListNode));
    ListNode *pst21 = (ListNode *)malloc(sizeof(ListNode));
    ListNode *pst31 = (ListNode *)malloc(sizeof(ListNode));
 

    pst11->val = 21;
    pst11->next = pst21;
    pst21->val = 69;
    pst21->next = pst31;
    pst31->val = 70;
    pst31->next = NULL;

    pst4 = mergeTwoLists(pst1,pst11);
    while(pst4){
        cout<<"values: "<<pst4->val<<endl;
        pst4 = pst4->next;
    }
    system("pause");

}

void removeNthFromEnd(){
    ListNode *pst1 = (ListNode *)malloc(sizeof(ListNode));
    ListNode *pst2 = (ListNode *)malloc(sizeof(ListNode));
    ListNode *pst3 = (ListNode *)malloc(sizeof(ListNode));
    ListNode *pst4;

    pst1->val = 1;
    pst1->next = pst2;
    pst2->val = 2;
    pst2->next = pst3;
    pst3->val = 3;
    pst3->next = NULL;

    pst4 = removeNthFromEnd(pst1, 3);
    system("pause");

}

void main()
{
    mergeTwoLists();

    system("pause");
}