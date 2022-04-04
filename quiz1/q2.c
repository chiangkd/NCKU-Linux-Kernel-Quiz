#include <stddef.h>

struct ListNode {
    int val;
    struct ListNode *next;
};

struct ListNode *deleteDuplicates(struct ListNode *head)
{
    if (!head)
        return NULL;

    if (head->next && head->val == head->next->val) {   //COND1
        /* Remove all duplicate numbers */
        while (head->next && head->val == head->next->val)  //COND2
            head = head->next;
        return deleteDuplicates(head->next);
    }

    head->next = deleteDuplicates(head->next);
    return head;
}