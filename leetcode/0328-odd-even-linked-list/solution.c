/* ============================================================================
 * LeetCode #328: Odd Even Linked List (Medium)
 * URL:      https://leetcode.com/problems/odd-even-linked-list/
 * Language: C
 *
 * What is being built / Approach:
 *   Groups all odd-indexed nodes together followed by the even-indexed nodes
 *   in-place in O(1) extra space and O(n) time.
 *   Time Complexity: O(n) | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: head = [1, 2, 3, 4, 5]       -> [1, 3, 5, 2, 4]
 *   - Example 2: head = [2, 1, 3, 5, 6, 4, 7] -> [2, 3, 6, 7, 1, 5, 4]
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

struct ListNode
{
    int val;
    struct ListNode *next;
};

static struct ListNode *createList(const int *vals, int n)
{
    struct ListNode *head = NULL;
    struct ListNode *tail = NULL;
    for(int i = 0; i < n; i++)
    {
        struct ListNode *node = (struct ListNode *)malloc(sizeof(struct ListNode));
        node->val = vals[i];
        node->next = NULL;
        if(head == NULL)
        {
            head = node;
            tail = node;
        }
        else
        {
            tail->next = node;
            tail = node;
        }
    }
    return head;
}

static bool assertListEquals(struct ListNode *head, const int *expected, int n)
{
    struct ListNode *curr = head;
    for(int i = 0; i < n; i++)
    {
        if(curr == NULL || curr->val != expected[i])
        {
            return false;
        }
        curr = curr->next;
    }
    return (curr == NULL);
}

static void printList(struct ListNode *head)
{
    printf("[");
    struct ListNode *curr = head;
    while(curr != NULL)
    {
        printf("%d", curr->val);
        if(curr->next != NULL)
        {
            printf(", ");
        }
        curr = curr->next;
    }
    printf("]");
}

static void freeList(struct ListNode *head)
{
    while(head != NULL)
    {
        struct ListNode *tmp = head;
        head = head->next;
        free(tmp);
    }
}

struct ListNode *oddEvenList(struct ListNode *head)
{
    if(head == NULL)
    {
        return NULL;
    }
    else
    {
        struct ListNode *odd = head;
        struct ListNode *even = head->next;
        struct ListNode *evenHead = head->next;

        while(even && even->next)
        {
            odd->next = odd->next->next;
            even->next = even->next->next;
            odd = odd->next;
            even = even->next;
        }
        odd->next = evenHead;
        return head;
    }
}

int main(void)
{
    printf("=== LeetCode #328: Odd Even Linked List ===\n");

    int a1[] = {1, 2, 3, 4, 5}, exp1[] = {1, 3, 5, 2, 4};
    struct ListNode *l1 = oddEvenList(createList(a1, 5));
    printf("Example 1: [1,2,3,4,5]     -> ");
    printList(l1);
    printf("\n");
    assert(assertListEquals(l1, exp1, 5));
    freeList(l1);

    int a2[] = {2, 1, 3, 5, 6, 4, 7}, exp2[] = {2, 3, 6, 7, 1, 5, 4};
    struct ListNode *l2 = oddEvenList(createList(a2, 7));
    printf("Example 2: [2,1,3,5,6,4,7] -> ");
    printList(l2);
    printf("\n");
    assert(assertListEquals(l2, exp2, 7));
    freeList(l2);

    printf("All tests passed!\n");
    return 0;
}
