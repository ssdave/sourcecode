/**
 * @file main.c
 * @brief RTOS Top-Half ISR & Bottom-Half Deferred Processing Test Harness.
 */

#include "rtos_sensor.h"
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct
{
  sensor_pipeline_t *pipe;
  sensor_mmio_regs_t *hw_regs;
  int num_samples;
  int delay_us;
} irq_generator_arg_t;

static void *simulated_hardware_irq_thread(void *arg)
{
  irq_generator_arg_t *irq_arg = (irq_generator_arg_t *)arg;
  for(int i = 0; i < irq_arg->num_samples; i++)
  {
    usleep((useconds_t)irq_arg->delay_us);

    /* Hardware generates a new sample and raises DRDY interrupt flag */
    irq_arg->hw_regs->data = (uint16_t)(1000 + i * 100);
    irq_arg->hw_regs->status |= SENSOR_STATUS_DRDY;

    /* Hardware invokes Top-Half ISR */
    bool higher_woken = false;
    sensor_drdy_isr(irq_arg->pipe, &higher_woken);
  }
  return NULL;
}

int main(void)
{
  printf("=== Running RTOS Sensor Pipeline Driver Verification ===\n");

  /* Probe for scaffold state */
  spsc_ring_t test_ring;
  if(spsc_ring_init(&test_ring) == -1)
  {
    printf("[Exercise Scaffold] spsc_ring_init() returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    return 0;
  }

  printf("[Verification 1] Testing SPSC ring buffer FIFO ordering & boundaries...\n");
  uint16_t popped_val = 0;
  assert(spsc_ring_pop(&test_ring, &popped_val) == -1); /* Pop from empty */

  assert(spsc_ring_push(&test_ring, 100) == 0);
  assert(spsc_ring_push(&test_ring, 200) == 0);
  assert(spsc_ring_push(&test_ring, 300) == 0);

  assert(spsc_ring_pop(&test_ring, &popped_val) == 0 && popped_val == 100);
  assert(spsc_ring_pop(&test_ring, &popped_val) == 0 && popped_val == 200);
  assert(spsc_ring_pop(&test_ring, &popped_val) == 0 && popped_val == 300);
  assert(spsc_ring_pop(&test_ring, &popped_val) == -1); /* Empty again */

  printf("[Verification 2] Testing SPSC ring buffer overflow & non-blocking drops...\n");
  spsc_ring_init(&test_ring);
  for(uint16_t i = 0; i < SPSC_RING_CAPACITY; i++)
  {
    assert(spsc_ring_push(&test_ring, (uint16_t)(10 + i)) == 0);
  }
  /* Ring is now full. Extra push must fail and increment dropped counter */
  assert(spsc_ring_push(&test_ring, 999) == -1);
  assert(test_ring.dropped == 1);

  /* Drain ring and verify contents are intact */
  for(uint16_t i = 0; i < SPSC_RING_CAPACITY; i++)
  {
    assert(spsc_ring_pop(&test_ring, &popped_val) == 0);
    assert(popped_val == (10 + i));
  }
  assert(spsc_ring_pop(&test_ring, &popped_val) == -1);

  printf("[Verification 3] Testing sensor pipeline initialization & mock MMIO binding...\n");
  sensor_mmio_regs_t mock_hw;
  memset(&mock_hw, 0, sizeof(mock_hw));
  mock_hw.cr = SENSOR_CR_IE | SENSOR_CR_START;

  sensor_pipeline_t pipe;
  int init_rc = sensor_pipeline_init(&pipe, &mock_hw);
  if(init_rc == -1)
  {
    printf("[Exercise Scaffold] sensor_pipeline_init() returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    return 0;
  }
  assert(pipe.hw_regs == &mock_hw);

  printf("[Verification 4] Testing Top-Half ISR (sensor_drdy_isr) execution...\n");
  /* Hardware raises interrupt with sample */
  mock_hw.data = 1234;
  mock_hw.status = SENSOR_STATUS_DRDY;

  bool higher_woken = false;
  sensor_drdy_isr(&pipe, &higher_woken);

  /* Verify ISR acknowledged/cleared DRDY flag in MMIO status register */
  assert((mock_hw.status & SENSOR_STATUS_DRDY) == 0);
  assert(higher_woken == true);

  /* Probe for Bottom-Half implementation */
  if(sensor_process_next_sample(&pipe, 0) == -1)
  {
    printf("[Exercise Scaffold] sensor_process_next_sample() returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    sensor_pipeline_destroy(&pipe);
    return 0;
  }

  /* Verify sample was processed */
  assert(pipe.processed_count == 1);
  assert(pipe.latest_avg == 1234);
  /* Verify bottom-half re-enabled hardware interrupts */
  assert((mock_hw.cr & SENSOR_CR_IE) != 0);

  printf("[Verification 5] Testing Bottom-Half moving average filter...\n");
  /* Feed 3 more samples: 2000, 3000, 4000 */
  uint16_t stream[3] = {2000, 3000, 4000};
  for(int i = 0; i < 3; i++)
  {
    mock_hw.data = stream[i];
    mock_hw.status = SENSOR_STATUS_DRDY;
    higher_woken = false;
    sensor_drdy_isr(&pipe, &higher_woken);
    assert(sensor_process_next_sample(&pipe, 50) == 0);
  }

  /* Window has: 1234, 2000, 3000, 4000 -> Sum = 10234, Avg = 10234 / 4 = 2558 */
  assert(pipe.processed_count == 4);
  assert(pipe.latest_avg == (1234 + 2000 + 3000 + 4000) / 4);

  /* Feed 5th sample: 5000 (evicts 1234) */
  mock_hw.data = 5000;
  mock_hw.status = SENSOR_STATUS_DRDY;
  sensor_drdy_isr(&pipe, &higher_woken);
  assert(sensor_process_next_sample(&pipe, 50) == 0);
  /* Window has: 5000, 2000, 3000, 4000 -> Sum = 14000, Avg = 3500 */
  assert(pipe.processed_count == 5);
  assert(pipe.latest_avg == 3500);

  sensor_pipeline_destroy(&pipe);

  printf(
      "[Verification 6] Testing concurrent asynchronous IRQ producer vs. RTOS task consumer...\n");
  memset(&mock_hw, 0, sizeof(mock_hw));
  mock_hw.cr = SENSOR_CR_IE | SENSOR_CR_START;
  assert(sensor_pipeline_init(&pipe, &mock_hw) == 0);

  const int TOTAL_TEST_SAMPLES = 20;
  irq_generator_arg_t irq_arg = {
      .pipe = &pipe,
      .hw_regs = &mock_hw,
      .num_samples = TOTAL_TEST_SAMPLES,
      .delay_us = 1000 /* 1 ms per interrupt */
  };

  pthread_t irq_thread;
  assert(pthread_create(&irq_thread, NULL, simulated_hardware_irq_thread, &irq_arg) == 0);

  /* Consumer task thread loop */
  int consumed = 0;
  while(consumed < TOTAL_TEST_SAMPLES)
  {
    int rc = sensor_process_next_sample(&pipe, 100);
    assert(rc == 0);
    consumed++;
  }

  pthread_join(irq_thread, NULL);
  assert(pipe.processed_count == TOTAL_TEST_SAMPLES);
  assert(pipe.ring.dropped == 0);

  printf("[Verification 7] Testing invalid arguments and error states...\n");
  assert(spsc_ring_init(NULL) == -1);
  assert(spsc_ring_push(NULL, 100) == -1);
  assert(spsc_ring_pop(NULL, &popped_val) == -1);
  assert(spsc_ring_pop(&pipe.ring, NULL) == -1);
  assert(sensor_pipeline_init(NULL, &mock_hw) == -1);
  assert(sensor_pipeline_init(&pipe, NULL) == -1);
  assert(sensor_process_next_sample(NULL, 10) == -1);

  sensor_pipeline_destroy(&pipe);

  printf("=== All RTOS Sensor Pipeline Driver tests passed successfully! ===\n");
  return 0;
}
