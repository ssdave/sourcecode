/* ============================================================================
 * LeetCode #92: Reverse Linked List II (Medium)
 * URL:      https://leetcode.com/problems/reverse-linked-list-ii/
 * Language: C
 *
 * What is being built / Approach:
 *   Reverses the sublist of a singly-linked list from 1-indexed position `m`
 *   to `n` in-place and splices the reversed segment back into the list.
 *   Time Complexity: O(n) | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: head = [1, 2, 3, 4, 5], left = 2, right = 4 -> [1, 4, 3, 2, 5]
 *   - Example 2: head = [5],             left = 1, right = 1 -> [5]
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

struct ListNode *reverse(struct ListNode *head, int m, int n, struct ListNode *prevNode)
{
    int count = n - m;
    int loop = 0;

    struct ListNode *prev = NULL;
    struct ListNode *curr = head;
    struct ListNode *next = curr->next;

    while(loop != count)
    {
        curr->next = prev;
        prev = curr;
        curr = next;
        next = curr->next;
        loop++;
    }
    curr->next = prev;
    if(m != 1)
    {
        prevNode->next = curr;
    }
    head->next = next;
    if(m == 1)
    {
        return curr;
    }
    else
    {
        return NULL;
    }
}

struct ListNode *reverseBetween(struct ListNode *head, int m, int n)
{
    int count = 1;
    struct ListNode *iter = head;
    struct ListNode *prev = NULL;

    if(!head)
    {
        return NULL;
    }

    while(count != m)
    {
        if(count == (m - 1))
        {
            prev = iter;
        }
        iter = iter->next;
        count++;
    }

    if(m == 1)
    {
        head = reverse(iter, m, n, prev);
    }
    else
    {
        reverse(iter, m, n, prev);
    }

    return head;
}

int main(void)
{
    printf("=== LeetCode #92: Reverse Linked List II ===\n");

    int a1[] = {1, 2, 3, 4, 5}, exp1[] = {1, 4, 3, 2, 5};
    struct ListNode *l1 = reverseBetween(createList(a1, 5), 2, 4);
    printf("Example 1: [1,2,3,4,5], left=2, right=4 -> ");
    printList(l1);
    printf("\n");
    assert(assertListEquals(l1, exp1, 5));
    freeList(l1);

    int a2[] = {5}, exp2[] = {5};
    struct ListNode *l2 = reverseBetween(createList(a2, 1), 1, 1);
    printf("Example 2: [5],         left=1, right=1 -> ");
    printList(l2);
    printf("\n");
    assert(assertListEquals(l2, exp2, 1));
    freeList(l2);

    printf("All tests passed!\n");
    return 0;
}
