/* ============================================================================
 * File: camera_isp_driver.c (Part 1 Implemented; Parts 2-5 Practice Stubs)
 *
 * Status:
 *   - Part 1 (`update_camera_register`): Implemented and passing (`0x12345678` ->
 *     `0x123A5678`).
 *   - Parts 2-5 (`enqueue_frame_force`, `dequeue_frame`, `custom_memmove`,
 *     `poll_pmu_boot_sequence`): Unimplemented starter stubs for practice.
 *
 * What is being built:
 *   Embedded Camera ISP / Firmware Driver Subsystems:
 *     1. `update_camera_register()` — Safe read-modify-write update of bits 12..15
 *        in a `volatile uint32_t` hardware register.
 *     2. `enqueue_frame_force()` & `dequeue_frame()` — Lock-free VSYNC frame
 *        ring buffer (`RING_SIZE = 4`) that drops and frees the oldest frame
 *        on overflow to prevent memory leaks.
 *     3. `custom_memmove()` — Word-aligned memory copy handling overlapping
 *        source and destination regions safely.
 *     4. `poll_pmu_boot_sequence()` — Non-blocking PMU power-rail sequencing
 *        state machine (`IOVDD -> AVDD -> DVDD`) with 10ms inter-rail delays.
 *
 * What is being tested:
 *   4-part driver verification harness testing bitfield masking (`0x12345678` ->
 *   `0x123A5678`), 5-frame overflow handling on a 4-slot queue, overlapping
 *   forward buffer copy (`"abcdefghijklmnop"`), and 5ms-tick non-blocking PMU
 *   state transitions.
 * ============================================================================ */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// MOCKS & STUBS (Simulating Hardware)
// ============================================================================
uint32_t simulated_hardware_register = 0x12345678;

void i2c_write(uint8_t addr, uint8_t reg, uint8_t val)
{
  printf("   [Hardware I2C] -> Addr: 0x%02X, Reg: 0x%02X, Val: 0x%02X\n", addr, reg, val);
}

// ============================================================================
// 1. REGISTER MANIPULATION (Bit-Banging)
// ============================================================================
// Goal: Update ONLY bits 12-15 of the register with new_value.
// Constraint: Do not alter any other bits. Defensively ensure new_value fits in 4 bits.
void update_camera_register(volatile uint32_t *reg_addr, uint32_t new_value)
{
  // Mask new_value to 4 bits (0xF) so out-of-range bits cannot corrupt neighboring fields
  new_value &= 0xF;
  // Read current 32-bit register value, clear bits [15:12] (~0xF000), and OR in shifted new_value
  uint32_t old_value = *reg_addr;
  old_value &= ~(0xF000);
  old_value |= (new_value << 12);
  *reg_addr = old_value;
}

// ============================================================================
// 2 & 3. LOCK-FREE QUEUE & OVERFLOW LOGIC
// ============================================================================
#define RING_SIZE 4 // Power of 2 for fast modulo

typedef struct
{
  uint32_t frame_id;
} Frame;

typedef struct
{
  Frame *frames[RING_SIZE];
  volatile uint32_t write_idx;
  volatile uint32_t read_idx;
} LockFreeQueue;

LockFreeQueue vsync_queue = {0};

// Goal: Insert new_frame. If buffer is full, drop the OLDEST frame to make room.
// Constraint: Lock-free. Memory barriers required. Prevent memory leaks.
void enqueue_frame_force(LockFreeQueue *q, Frame *new_frame)
{
  // AI Review Feedback (Practice Exercise): Implement lock-free queue overflow handling here
}

// Goal: Pop and return the oldest frame. Return NULL if empty.
Frame *dequeue_frame(LockFreeQueue *q)
{
  // AI Review Feedback (Practice Exercise): Implement dequeue logic here
  return NULL;
}

// ============================================================================
// 4. HARDWARE-OPTIMIZED MEMMOVE
// ============================================================================
// Goal: Copy num_bytes from src to dest.
// Constraint 1: Must handle overlapping memory regions safely.
// Constraint 2: Optimize for 32-bit ARM (copy 4 bytes at a time where possible).
void custom_memmove(void *dest, const void *src, size_t num_bytes)
{
  // AI Review Feedback (Practice Exercise): Implement overlapping, word-aligned copy logic here
}

// ============================================================================
// 5. NON-BLOCKING PMU STATE MACHINE
// ============================================================================
#define PMU_ADDR 0x42
#define REG_IOVDD 0x10
#define REG_AVDD 0x11
#define REG_DVDD 0x12

typedef enum
{
  STATE_IDLE,
  STATE_ENABLE_IOVDD,
  STATE_ENABLE_AVDD,
  STATE_ENABLE_DVDD,
  STATE_DONE
} PmuState;

// Goal: Step through the power states. Trigger the next I2C write only if
// 10ms has passed since the last one.
// Constraint: Cannot use sleep() or block the CPU.
// Returns: 1 if sequence is STATE_DONE, 0 otherwise.
int poll_pmu_boot_sequence(uint64_t current_sys_time_ms)
{
  // AI Review Feedback (Practice Exercise): Implement non-blocking state machine here
  return 0;
}

// ============================================================================
// MAIN TEST HARNESS
// ============================================================================
int main()
{
  printf("--- 1. Testing Register Manipulation ---\n");
  printf("Original Register: 0x%08X\n", simulated_hardware_register);
  update_camera_register(&simulated_hardware_register, 0xA); // 1010
  printf("Updated Register:  0x%08X \n(Expected:         0x123A5678)\n\n",
         simulated_hardware_register);

  printf("--- 2 & 3. Testing Lock-Free Queue Overflow ---\n");
  for(int i = 1; i <= 5; i++)
  {
    Frame *f = malloc(sizeof(Frame));
    f->frame_id = i;
    printf("Enqueueing Frame %d...\n", i);
    enqueue_frame_force(&vsync_queue, f);
  }
  printf("Dequeueing remaining frames: ");
  Frame *dequeued;
  while((dequeued = dequeue_frame(&vsync_queue)) != NULL)
  {
    printf("%d ", dequeued->frame_id);
    free(dequeued);
  }
  printf("\n(Expected: 2 3 4 5)\n\n");

  printf("--- 4. Testing Custom Memmove (Overlap) ---\n");
  char buffer[20] = "abcdefghijklmnop";
  printf("Original buffer: %s\n", buffer);
  custom_memmove(&buffer[2], &buffer[0], 4);
  printf("Overlap buffer:  %s \n(Expected:        ababcdghijklmnop)\n\n", buffer);

  printf("--- 5. Testing PMU State Machine ---\n");
  uint64_t simulated_time = 0;
  int done = 0;
  int iterations = 0;
  while(!done && iterations < 100)
  {
    done = poll_pmu_boot_sequence(simulated_time);
    if(!done)
    {
      simulated_time += 5; // Advance clock by 5ms per loop
    }
    iterations++;
  }
  printf("PMU Sequence Complete at T=%llums. (Expected: T>=30ms)\n", simulated_time);

  return 0;
}