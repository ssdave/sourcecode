/* ============================================================================
 * LeetCode #206: Reverse Linked List (Easy)
 * URL:      https://leetcode.com/problems/reverse-linked-list/
 * Language: C
 *
 * What is being built / Approach:
 *   Reverses a singly-linked list iteratively in-place using three pointers
 *   (`prev`, `curr`, `next`).
 *   Time Complexity: O(n) | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: head = [1, 2, 3, 4, 5] -> [5, 4, 3, 2, 1]
 *   - Example 2: head = [1, 2]          -> [2, 1]
 *   - Example 3: head = []              -> []
 * ============================================================================ */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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

struct ListNode *reverseList(struct ListNode *head)
{
  if(!head)
  {
    return NULL;
  }
  else
  {
    struct ListNode *prev = NULL;
    struct ListNode *curr = head;
    struct ListNode *next = curr->next;

    while(curr->next != NULL)
    {
      curr->next = prev;
      prev = curr;
      curr = next;
      next = curr->next;
    }
    curr->next = prev;
    return curr;
  }
}

int main(void)
{
  printf("=== LeetCode #206: Reverse Linked List ===\n");

  int a1[] = {1, 2, 3, 4, 5}, exp1[] = {5, 4, 3, 2, 1};
  struct ListNode *l1 = reverseList(createList(a1, 5));
  printf("Example 1: [1,2,3,4,5] -> ");
  printList(l1);
  printf("\n");
  assert(assertListEquals(l1, exp1, 5));
  freeList(l1);

  int a2[] = {1, 2}, exp2[] = {2, 1};
  struct ListNode *l2 = reverseList(createList(a2, 2));
  printf("Example 2: [1,2]       -> ");
  printList(l2);
  printf("\n");
  assert(assertListEquals(l2, exp2, 2));
  freeList(l2);

  struct ListNode *l3 = reverseList(NULL);
  printf("Example 3: []          -> ");
  printList(l3);
  printf("\n");
  assert(l3 == NULL);

  printf("All tests passed!\n");
  return 0;
}
