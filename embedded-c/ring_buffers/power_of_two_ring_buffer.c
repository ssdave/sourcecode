/* ============================================================================
 * File: power_of_two_ring_buffer.c
 *
 * What is being built:
 *   Static power-of-two circular queue (`BUFFER_SIZE = 8`) using `volatile`
 *   head/tail indices and bitwise AND masking (`(index + 1) & BUFFER_MASK`)
 *   instead of modulo division for single-cycle index wrap-around.
 *
 * What is being tested:
 *   Enqueues integers up to the full capacity (`BUFFER_SIZE - 1` usable slots),
 *   verifies overflow rejection when `((head + 1) & BUFFER_MASK) == tail`,
 *   dequeues elements in FIFO order, and verifies empty-buffer detection.
 * ============================================================================ */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 8
#define BUFFER_MASK (BUFFER_SIZE - 1)

int buffer[BUFFER_SIZE];
volatile uint16_t head;
volatile uint16_t tail;

bool isEmpty(void)
{
  // Queue is empty when head and tail point to the same slot
  if(head == tail)
  {
    return true;
  }

  return false;
}

bool isFull(void)
{
  // One slot is reserved to distinguish full from empty: full when advancing head
  // by 1 (modulo BUFFER_SIZE via bitwise AND with BUFFER_MASK) collides with tail
  if(((head + 1) & BUFFER_MASK) == tail)
  {
    return true;
  }

  return false;
}

void enqueue(int value)
{
  if(isFull())
  {
    return;
  }

  // Store item at head and wrap head index using power-of-two bitmask
  buffer[head] = value;
  head = (head + 1) & BUFFER_MASK;
}

int dequeue(void)
{
  if(isEmpty())
  {
    return -1;
  }

  // Read oldest item at tail and wrap tail index using power-of-two bitmask
  int ret = buffer[tail];
  tail = (tail + 1) & BUFFER_MASK;

  return ret;
}

int main()
{
  // 1. Fill the buffer
  for(int i = 1; i <= (BUFFER_SIZE - 1); i++)
  {
    enqueue(i * 10);
  }

  // 2. Try to overflow it (This should be safely ignored)
  enqueue(80);

  // 3. Verify it is full
  if(isFull())
  {
    printf("Buffer is full as expected.\n");
  }

  // 4. Drain the buffer
  while(!isEmpty())
  {
    printf("Dequeued: %d\n", dequeue());
  }

  // 5. Try to underflow it (Should return -1)
  printf("Empty dequeue attempt: %d\n", dequeue());

  return 0;
}