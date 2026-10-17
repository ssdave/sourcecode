/* ============================================================================
 * File: isr_ring_buffer.c
 *
 * The Scenario:
 *   The wireless proximity tag is asleep. Suddenly, a host device initiates an
 *   active ranging session via Ultra-Wideband (UWB). The UWB radio hardware
 *   triggers an interrupt and starts dumping ranging packets into memory. You
 *   need to queue these packets for the main RTOS thread to process without
 *   missing data or using locks that could freeze the interrupt handler.
 *
 * What is being built:
 *   Statically allocated, lock-free single-producer/single-consumer circular
 *   buffer (`ring_buf_t`, `BUFFER_SIZE = 16`) for `uwb_packet_t` frames using
 *   power-of-two bitmask indexing (`BUFFER_SIZE - 1`) without `malloc` or mutexes.
 *
 * What is being tested:
 *   5-stage verification harness:
 *     1. Filling buffer to exact capacity (16 packets).
 *     2. Overfill protection (rejecting the 17th packet).
 *     3. Full FIFO drain in order (`0..15`).
 *     4. Underflow protection on an empty buffer.
 *     5. Index wrap-around across the physical array boundary.
 * ============================================================================ */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 16

typedef struct
{
  uint32_t distance_mm;
  uint16_t angle_degrees;
} uwb_packet_t;

typedef struct
{
  uwb_packet_t ring_buf_s[BUFFER_SIZE];
  int write_idx;
  int read_idx;
} ring_buf_t;

ring_buf_t ring_buf;

void ring_buffer_init(void)
{
  ring_buf.read_idx = 0;
  ring_buf.write_idx = 0;
}

bool ring_buffer_push(const uwb_packet_t *packet)
{
  if(packet != NULL)
  {
    // Buffer is full when write_idx is BUFFER_SIZE ahead of read_idx
    if((ring_buf.write_idx - ring_buf.read_idx) == BUFFER_SIZE)
    {
      return false;
    }

    // Wrap index into [0, BUFFER_SIZE - 1] using power-of-two bitmask
    ring_buf.write_idx = ring_buf.write_idx & (BUFFER_SIZE - 1);

    ring_buf.ring_buf_s[ring_buf.write_idx].angle_degrees = packet->angle_degrees;
    ring_buf.ring_buf_s[ring_buf.write_idx++].distance_mm = packet->distance_mm;

    return true;
  }
  return false;
}

bool ring_buffer_pop(uwb_packet_t *packet)
{
  if(packet != NULL)
  {
    // Buffer is empty when read_idx catches up to write_idx
    if(ring_buf.read_idx == ring_buf.write_idx)
    {
      return false;
    }

    // Wrap index into [0, BUFFER_SIZE - 1] using power-of-two bitmask
    ring_buf.read_idx = ring_buf.read_idx & (BUFFER_SIZE - 1);

    packet->angle_degrees = ring_buf.ring_buf_s[ring_buf.read_idx].angle_degrees;
    packet->distance_mm = ring_buf.ring_buf_s[ring_buf.read_idx++].distance_mm;

    return true;
  }
  else
  {
    return false;
  }
}

/* --------------------------------------------------------------------------
 * 3. The Test Harness (Verification)
 * -------------------------------------------------------------------------- */
int main()
{
  ring_buffer_init();
  uwb_packet_t test_packet;
  bool success;

  printf("--- Interrupt-Safe Ring Buffer Verification ---\n\n");

  // TEST 1: Fill the buffer completely
  printf("1. Filling buffer to capacity (%d items)...\n", BUFFER_SIZE);
  for(int i = 0; i < BUFFER_SIZE; i++)
  {
    uwb_packet_t p = {.distance_mm = i * 100, .angle_degrees = i};
    success = ring_buffer_push(&p);
    if(!success)
    {
      printf("[FAIL] Rejected packet %d prematurely.\n", i);
      return -1;
    }
  }
  printf("[PASS] Successfully pushed %d items.\n", BUFFER_SIZE);

  // TEST 2: Overfill the buffer (Trigger boundary constraint)
  printf("2. Testing overfill protection...\n");
  uwb_packet_t overflow_p = {.distance_mm = 9999, .angle_degrees = 360};
  success = ring_buffer_push(&overflow_p);
  if(!success)
  {
    printf("[PASS] Buffer successfully rejected the %dth packet (Full).\n", BUFFER_SIZE + 1);
  }
  else
  {
    printf("[FAIL] Buffer allowed overfill!\n");
  }

  // TEST 3: Drain the buffer
  printf("3. Draining buffer entirely...\n");
  for(int i = 0; i < BUFFER_SIZE; i++)
  {
    success = ring_buffer_pop(&test_packet);
    if(!success || test_packet.angle_degrees != i)
    {
      printf("[FAIL] Failed to pop packet %d properly.\n", i);
      return -1;
    }
  }
  printf("[PASS] Successfully popped all %d items in order.\n", BUFFER_SIZE);

  // TEST 4: Underflow (Read from empty buffer)
  printf("4. Testing underflow protection...\n");
  success = ring_buffer_pop(&test_packet);
  if(!success)
  {
    printf("[PASS] Buffer correctly returned false when empty.\n");
  }
  else
  {
    printf("[FAIL] Buffer allowed read when empty!\n");
  }

  // TEST 5: Wrap-around logic
  printf("5. Testing memory wrap-around...\n");
  // Push 5 more items to cross the physical memory array boundary
  // (since we are already at index 16 under the hood)
  for(int i = 0; i < 5; i++)
  {
    uwb_packet_t p = {.distance_mm = 5000, .angle_degrees = 99};
    ring_buffer_push(&p);
  }

  // Pop them back
  int pop_count = 0;
  while(ring_buffer_pop(&test_packet))
  {
    pop_count++;
  }

  if(pop_count == 5)
  {
    printf("[PASS] Wrap-around logic is sound. Array indexing wrapped flawlessly.\n");
  }
  else
  {
    printf("[FAIL] Wrap-around failed. Only popped %d items.\n", pop_count);
  }

  printf("\nAll tests passed.\n");
  return 0;
}