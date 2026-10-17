/* ============================================================================
 * File: singly_linked_list.c
 *
 * What is being built:
 *   Dynamic singly linked list tail insertion (`append_node`) and traversal
 *   (`print_list`) using heap-allocated `ListNode` structures.
 *
 * What is being tested:
 *   Allocates a head node (`1`), appends nodes `2`, `3`, and `4` to the tail of
 *   the list, and traverses the list to print node values.
 *
 * Edge cases covered:
 *   - `append_node(NULL, ...)` is a safe no-op.
 *   - Appending to a single-node list (head with `next == NULL`).
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>

typedef struct ListNode
{
  int value;
  struct ListNode *next;
} ListNode;

void append_node(ListNode *head, int value)
{
  if(head == NULL)
  {
    return;
  }

  // Walk to the current tail node (the node whose next pointer is NULL)
  while(head->next != NULL)
  {
    head = head->next;
  }

  // AI Review Feedback: malloc() return address is not checked for NULL before dereferencing.
  ListNode *new_node = (ListNode *)malloc(sizeof(ListNode));
  new_node->value = value;
  new_node->next = NULL;

  head->next = new_node;
}

void print_list(ListNode *head)
{
  // AI Review Feedback: `head->next != NULL` terminates one node early, omitting the
  // tail value (4); changing to `while(head != NULL)` traverses the entire list.
  while(head->next != NULL)
  {
    printf("Value %d\t", head->value);
    head = head->next;
  }
}

int main()
{
  printf("--- Singly Linked List: append + traverse ---\n");

  ListNode *head = (ListNode *)malloc(sizeof(ListNode));
  head->value = 1;
  head->next = NULL;

  append_node(head, 2);

  append_node(head, 3);

  append_node(head, 4);

  print_list(head);
  printf("\n");

  // AI Review Feedback: Allocated list nodes are not freed before process termination.
  // Consider implementing a free_list() helper to prevent memory leaks in long-running processes.
  return 0;
}