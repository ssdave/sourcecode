/* ============================================================================
 * File: double_pointer.c
 *
 * What is being built:
 *   Double-pointer (`T **`) memory management, linked-list head mutation, and
 *   string tokenizer cursor advancement:
 *     1. `allocate_buffer(uint8_t **out_ptr, size_t size)` — Safely allocates
 *        heap memory and writes the allocated address into the caller's pointer.
 *     2. `list_push(Node **head, int val)` — Prepends a node to a singly linked
 *        list by updating `*head` directly without returning a new head pointer.
 *     3. `next_token(char **cursor, char delim)` — Extracts the next delimiter-
 *        separated token and advances the caller's `*cursor` pointer past `delim`.
 *
 * What is being tested:
 *   Assertion suite testing NULL/zero-size allocation guards, LIFO head
 *   insertion (`10 -> 20 -> 30`), stateful CSV token extraction (`"CMD,SET,100"`),
 *   and end-of-stream `NULL` termination.
 * ============================================================================ */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node
{
  int val;
  struct Node *next;
} Node;

/**
 * 1. Allocate 'size' bytes using malloc and assign the allocated address to *out_ptr.
 *    Returns true on success, false if out_ptr is NULL or malloc fails.
 */
bool allocate_buffer(uint8_t **out_ptr, size_t size)
{
  // Guard against NULL output pointer or zero-byte allocation request
  if((out_ptr == NULL) || (size == 0))
    return false;

  // Dereference out_ptr once to write the heap address into the caller's pointer variable
  *out_ptr = malloc(sizeof(uint8_t) * size);

  if(*out_ptr == NULL)
    return false;

  return true;
}

/**
 * 2. Prepend a new node with 'val' to the front of the linked list.
 *    Updates the caller's *head pointer to point to the newly allocated node.
 */
void list_push(Node **head, int val)
{
  if(head == NULL)
    return;

  // Link the new node in front of the current head (*head) and update *head in-place
  Node *temp = *head;
  Node *new = (Node *)malloc(sizeof(Node));
  new->val = val;
  new->next = temp;

  *head = new;
}

/**
 * 3. Extract the next token from *cursor up to 'delim' (replace delim with '\0').
 *    Advance *cursor to point to the character after delim (or to '\0' if at end).
 *    Returns the start of the token, or NULL if *cursor is empty/NULL.
 */
char *next_token(char **cursor, char delim)
{
  // Stop when cursor is NULL or already positioned at the terminating NUL byte
  if((cursor == NULL) || (**cursor == '\0'))
    return NULL;

  char *result = (char *)malloc(strlen(*cursor));
  char *start = result;

  // Copy characters until the delimiter or end-of-string is reached
  while(**cursor != delim)
  {
    strncpy(result, *cursor, 1);
    result++;
    (*cursor)++;
    if(**cursor == '\0')
      return start;
  }

  // Advance caller's cursor past the delimiter and NUL-terminate the extracted token
  *cursor = *cursor + 1;
  *result = '\0';
  return start;
}

int main(void)
{
  printf("--- Running Double Pointer (**ptr) Tests ---\n");

  /* 1. Test Buffer Allocation via Output Parameter */
  uint8_t *my_buf = NULL;
  assert(allocate_buffer(&my_buf, 64) == true);
  assert(my_buf != NULL);
  free(my_buf);
  assert(allocate_buffer(NULL, 64) == false); /* NULL safety */
  printf("[PASS] allocate_buffer with **ptr\n");

  /* 2. Test Linked List Head Updates */
  Node *head = NULL;
  list_push(&head, 10);
  list_push(&head, 20);
  list_push(&head, 30);
  /* List should now be: 30 -> 20 -> 10 -> NULL */
  assert(head != NULL && head->val == 30);
  assert(head->next->val == 20);
  assert(head->next->next->val == 10);
  assert(head->next->next->next == NULL);

  /* Free nodes */
  while(head != NULL)
  {
    Node *temp = head;
    head = head->next;
    free(temp);
  }
  printf("[PASS] list_push with **head\n");

  /* 3. Test String Tokenizer Cursor Advancement */
  char packet[] = "CMD,SET,100";
  char *cursor = packet;

  char *t1 = next_token(&cursor, ',');
  assert(strcmp(t1, "CMD") == 0);

  char *t2 = next_token(&cursor, ',');
  assert(strcmp(t2, "SET") == 0);

  char *t3 = next_token(&cursor, ',');
  assert(strcmp(t3, "100") == 0);

  assert(next_token(&cursor, ',') == NULL);
  printf("[PASS] next_token with **cursor\n");

  printf("--- All tests passed! ---\n");
  return 0;
}