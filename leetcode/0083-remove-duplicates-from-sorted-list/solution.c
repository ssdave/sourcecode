/* ============================================================================
 * LeetCode #83: Remove Duplicates from Sorted List (Easy)
 * URL:      https://leetcode.com/problems/remove-duplicates-from-sorted-list/
 * Language: C
 *
 * What is being built / Approach:
 *   Deletes duplicate nodes from a sorted singly-linked list in-place by
 *   unlinking and freeing nodes whose value matches their predecessor.
 *   Time Complexity: O(n) | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: head = [1, 1, 2]       -> [1, 2]
 *   - Example 2: head = [1, 1, 2, 3, 3] -> [1, 2, 3]
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

struct ListNode *deleteDuplicates(struct ListNode *head)
{
    if(head == NULL)
    {
        return NULL;
    }

    struct ListNode *iter = head;
    struct ListNode *prev = NULL;

    while(iter != NULL)
    {
        if(prev && (iter->val == prev->val))
        {
            struct ListNode *temp = iter;
            iter = iter->next;
            prev->next = iter;

            free(temp);
            continue;
        }
        else
        {
            prev = iter;
        }

        iter = iter->next;
    }

    return head;
}

int main(void)
{
    printf("=== LeetCode #83: Remove Duplicates from Sorted List ===\n");

    int a1[] = {1, 1, 2}, exp1[] = {1, 2};
    struct ListNode *l1 = deleteDuplicates(createList(a1, 3));
    printf("Example 1: [1, 1, 2]       -> ");
    printList(l1);
    printf("\n");
    assert(assertListEquals(l1, exp1, 2));
    freeList(l1);

    int a2[] = {1, 1, 2, 3, 3}, exp2[] = {1, 2, 3};
    struct ListNode *l2 = deleteDuplicates(createList(a2, 5));
    printf("Example 2: [1, 1, 2, 3, 3] -> ");
    printList(l2);
    printf("\n");
    assert(assertListEquals(l2, exp2, 3));
    freeList(l2);

    printf("All tests passed!\n");
    return 0;
}
