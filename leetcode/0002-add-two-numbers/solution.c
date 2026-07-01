/* ============================================================================
 * LeetCode #2: Add Two Numbers (Medium)
 * URL:      https://leetcode.com/problems/add-two-numbers/
 * Language: C
 *
 * What is being built / Approach:
 *   Adds two non-negative integers represented as reverse-order singly-linked
 *   lists digit-by-digit while propagating decimal carry.
 *   Time Complexity: O(max(m, n)) | Space Complexity: O(max(m, n))
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: l1 = [2, 4, 3], l2 = [5, 6, 4] -> [7, 0, 8]
 *   - Example 2: l1 = [0], l2 = [0]             -> [0]
 *   - Example 3: l1 = [9,9,9,9,9,9,9], l2 = [9,9,9,9] -> [8,9,9,9,0,0,0,1]
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

struct ListNode *addTwoNumbers(struct ListNode *l1, struct ListNode *l2)
{
    if((l1 == NULL) || (l2 == NULL))
    {
        return NULL;
    }

    int carry = 0;

    struct ListNode *out = (struct ListNode *)malloc(sizeof(struct ListNode));
    struct ListNode *head = out;
    out->val = 0;
    out->next = NULL;
    int count = 0;

    while((l1 != NULL) || (l2 != NULL))
    {
        int num1 = 0;
        int num2 = 0;

        if(l1)
        {
            num1 = l1->val;
        }

        if(l2)
        {
            num2 = l2->val;
        }

        int sum = num1 + num2 + carry;
        carry = (sum > 9) ? 1 : 0;
        sum = sum % 10;

        if(count == 0)
        {
            out->val = sum;
            count++;
        }
        else
        {
            struct ListNode *temp = (struct ListNode *)malloc(sizeof(struct ListNode));
            temp->val = sum;
            temp->next = NULL;
            out->next = temp;
            out = out->next;
        }

        if(l1)
        {
            l1 = l1->next;
        }
        if(l2)
        {
            l2 = l2->next;
        }
    }

    if(carry)
    {
        out->next = (struct ListNode *)malloc(sizeof(struct ListNode));
        out->next->val = 1;
        out->next->next = NULL;
    }

    return head;
}

int main(void)
{
    printf("=== LeetCode #2: Add Two Numbers ===\n");

    int a1[] = {2, 4, 3}, b1[] = {5, 6, 4}, exp1[] = {7, 0, 8};
    struct ListNode *l1 = createList(a1, 3);
    struct ListNode *l2 = createList(b1, 3);
    struct ListNode *res1 = addTwoNumbers(l1, l2);
    printf("Example 1: [2,4,3] + [5,6,4] -> ");
    printList(res1);
    printf("\n");
    assert(assertListEquals(res1, exp1, 3));
    freeList(l1);
    freeList(l2);
    freeList(res1);

    int a2[] = {0}, b2[] = {0}, exp2[] = {0};
    l1 = createList(a2, 1);
    l2 = createList(b2, 1);
    struct ListNode *res2 = addTwoNumbers(l1, l2);
    printf("Example 2: [0] + [0]         -> ");
    printList(res2);
    printf("\n");
    assert(assertListEquals(res2, exp2, 1));
    freeList(l1);
    freeList(l2);
    freeList(res2);

    int a3[] = {9, 9, 9, 9, 9, 9, 9}, b3[] = {9, 9, 9, 9}, exp3[] = {8, 9, 9, 9, 0, 0, 0, 1};
    l1 = createList(a3, 7);
    l2 = createList(b3, 4);
    struct ListNode *res3 = addTwoNumbers(l1, l2);
    printf("Example 3: [9,9,9,9,9,9,9] + [9,9,9,9] -> ");
    printList(res3);
    printf("\n");
    assert(assertListEquals(res3, exp3, 8));
    freeList(l1);
    freeList(l2);
    freeList(res3);

    printf("All tests passed!\n");
    return 0;
}
