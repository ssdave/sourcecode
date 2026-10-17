/* ============================================================================
 * File: counted_ring_buffer.c (Practice Harness / Starter Template)
 *
 * Note:
 *   This file is an unimplemented unit-test harness kept for timed practice
 *   (`ring_buf_init` is implemented; `ring_buf_is_empty`, `ring_buf_is_full`,
 *   `ring_buf_push`, and `ring_buf_pop` are starter stubs). See `power_of_two_ring_buffer.c`
 *   in this directory for the completed power-of-two circular buffer.
 *
 * What is being built:
 *   Struct-encapsulated byte ring buffer (`ring_buffer_t`, `RING_BUF_SIZE = 8`)
 *   tracking `head`, `tail`, and explicit `count` to utilize all 8 slots of a
 *   power-of-two array with bitmask wrap-around (`RING_BUF_MASK`).
 *
 * What is being tested:
 *   Unit test suite verifying initialization, empty/full predicates, push/pop
 *   FIFO ordering, full-capacity boundary behavior, overflow/underflow rejection,
 *   and multi-cycle wrap-around across the physical end of the backing array.
 * ============================================================================ */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define RING_BUF_SIZE 8 /* Must be a power of 2 */
#define RING_BUF_MASK (RING_BUF_SIZE - 1)

typedef struct
{
  uint8_t buffer[RING_BUF_SIZE];
  size_t head;  /* Next write position */
  size_t tail;  /* Next read position */
  size_t count; /* Number of bytes currently in buffer */
} ring_buffer_t;

/**
 * 1. Initialize the ring buffer to empty.
 */
void ring_buf_init(ring_buffer_t *rb)
{
  rb->head = 0;
  rb->tail = 0;
  rb->count = 0;
}

/**
 * 2. Returns true if buffer is empty, false otherwise.
 */
bool ring_buf_is_empty(const ring_buffer_t *rb)
{
  /* Implement here */
  (void)rb;
  return false;
}

/**
 * 3. Returns true if buffer is full, false otherwise.
 */
bool ring_buf_is_full(const ring_buffer_t *rb)
{
  /* Implement here */
  (void)rb;
  return false;
}

/**
 * 4. Pushes a byte into the buffer.
 * Returns true on success, or false if the buffer is full.
 */
bool ring_buf_push(ring_buffer_t *rb, uint8_t byte)
{
  /* Implement here */
  (void)rb;
  (void)byte;
  return false;
}

/**
 * 5. Pops a byte from the buffer and writes it into *byte.
 * Returns true on success, or false if the buffer is empty.
 */
bool ring_buf_pop(ring_buffer_t *rb, uint8_t *byte)
{
  /* Implement here */
  (void)rb;
  (void)byte;
  return false;
}

int main(void)
{
  printf("--- Running Ring Buffer Tests ---\n");

  ring_buffer_t rb;
  ring_buf_init(&rb);

  /* 1. Initial state checks */
  assert(ring_buf_is_empty(&rb) == true);
  assert(ring_buf_is_full(&rb) == false);

  /* Pop on empty buffer must fail */
  uint8_t val = 0;
  assert(ring_buf_pop(&rb, &val) == false);
  printf("[PASS] Empty buffer checks\n");

  /* 2. Basic push and pop */
  assert(ring_buf_push(&rb, 0xAA) == true);
  assert(ring_buf_push(&rb, 0xBB) == true);
  assert(ring_buf_is_empty(&rb) == false);

  assert(ring_buf_pop(&rb, &val) == true && val == 0xAA);
  assert(ring_buf_pop(&rb, &val) == true && val == 0xBB);
  assert(ring_buf_is_empty(&rb) == true);
  printf("[PASS] Basic FIFO push/pop\n");

  /* 3. Fill buffer to capacity (8 bytes) */
  for(uint8_t i = 1; i <= RING_BUF_SIZE; i++)
  {
    assert(ring_buf_push(&rb, i) == true);
  }
  assert(ring_buf_is_full(&rb) == true);

  /* Push on full buffer must fail */
  assert(ring_buf_push(&rb, 0xFF) == false);
  printf("[PASS] Full buffer enforcement\n");

  /* 4. Drain buffer completely */
  for(uint8_t i = 1; i <= RING_BUF_SIZE; i++)
  {
    assert(ring_buf_pop(&rb, &val) == true);
    assert(val == i);
  }
  assert(ring_buf_is_empty(&rb) == true);
  printf("[PASS] Draining full buffer\n");

  /* 5. Circular wrap-around check */
  /* Push 5 bytes, pop 5 bytes to advance head/tail near end of array */
  for(uint8_t i = 0; i < 5; i++)
  {
    assert(ring_buf_push(&rb, i) == true);
    assert(ring_buf_pop(&rb, &val) == true);
  }
  /* Now fill again to force index wrap past index 7 back to index 0 */
  for(uint8_t i = 10; i < 10 + RING_BUF_SIZE; i++)
  {
    assert(ring_buf_push(&rb, i) == true);
  }
  assert(ring_buf_is_full(&rb) == true);

  for(uint8_t i = 10; i < 10 + RING_BUF_SIZE; i++)
  {
    assert(ring_buf_pop(&rb, &val) == true);
    assert(val == i);
  }
  assert(ring_buf_is_empty(&rb) == true);
  printf("[PASS] Circular wrap-around\n");

  printf("--- All tests passed! ---\n");
  return 0;
}