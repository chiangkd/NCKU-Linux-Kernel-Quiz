/* iteration method */
struct ListNode *deleteDuplicates(struct ListNode *head)
{
    if(!head) return NULL;
    
    bool head_dup = false;
    while(head->next && head->val == head->next->val){
        head_dup = true;
        head = head->next;
    }
    struct ListNode *c = head;
    for(struct ListNode *n = head->next ; n ;n = n-> next) {
        if(n-> next && n->val == n->next -> val) {
            struct ListNode* np = n->next;
            while(np && n->val == np->val) {
                np = np->next;
                n ->next = np;
                if(np && np->next && np->val == np->next->val) {
                    np = np->next;
                    n = np;
                }
            }
            n = n->next;
            c ->next = n;
        }
        c->next = n;
        c = n;
        if(!n)  // make sure for end is not NULL
            break;
    }
    return head_dup ? head->next : head;
}