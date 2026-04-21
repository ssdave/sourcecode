/* ============================================================================
 * File: Hello.c
 *
 * What is being built:
 *   Dynamic singly linked list construction (`addNode`) and traversal
 *   (`printNodes`) using heap-allocated `node` structures.
 *
 * What is being tested:
 *   Allocates a head node (`1`), appends nodes `2`, `3`, and `4` to the tail of
 *   the list, and traverses the list to print node values.
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>

typedef struct node
{
  int value;
  struct node *next;
} node;

void addNode(node *head, int value)
{
  if(head == NULL)
    return;

  while(head->next != NULL)
  {
    head = head->next;
  }

  node *temp = (node *)malloc(sizeof(node));
  temp->value = value;
  temp->next = NULL;

  head->next = temp;
}

void printNodes(node *head)
{
  while(head->next != NULL)
  {
    printf("Value %d\t", head->value);
    head = head->next;
  }
}

int main()
{
  printf("Hello, World!\n");

  node *head = (node *)malloc(sizeof(node));
  head->value = 1;
  head->next = NULL;

  addNode(head, 2);

  addNode(head, 3);

  addNode(head, 4);

  printNodes(head);

  return 0;
}