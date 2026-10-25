/**
 * @file sensor_pipeline.c
 * @brief Top-Half (ISR) and Bottom-Half (RTOS Task) Sensor Processing Implementation.
 */

#include "rtos_sensor.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Initializes the sensor pipeline context, binds hardware MMIO registers,
 * initializes the SPSC ring buffer, and prepares synchronization primitives.
 *
 * @param pipe Pointer to the sensor pipeline context structure.
 * @param hw_regs Pointer to the hardware MMIO register block.
 * @return 0 on success, or -1 on invalid argument or initialization failure.
 */
int sensor_pipeline_init(sensor_pipeline_t *pipe, sensor_mmio_regs_t *hw_regs)
{
  /* TODO: Author Implementation */
  if((pipe == NULL) || (hw_regs == NULL))
  {
    return -1;
  }
  if(spsc_ring_init(&pipe->ring) != 0)
  {
    return -1;
  }
  if(rtos_sem_init(&pipe->data_sem, 0) != 0)
  {
    return -1;
  }

  pipe->hw_regs = hw_regs;
  pipe->latest_avg = 0;
  pipe->processed_count = 0;
  pipe->window_count = 0;
  pipe->window_idx = 0;
  for(int i = 0; i < MOVING_AVG_WINDOW; i++)
  {
    pipe->window[i] = 0;
  }

  return 0;
}

/**
 * Top-half interrupt service routine triggered on hardware data-ready (DRDY).
 * Reads the hardware MMIO sample, acknowledges/clears the hardware interrupt,
 * enqueues the sample non-blockingly into the pipeline ring, and signals the RTOS.
 *
 * @param pipe Pointer to the sensor pipeline context structure.
 * @param higher_priority_task_woken Pointer to RTOS flag set if context switch requested.
 */
void sensor_drdy_isr(sensor_pipeline_t *pipe, bool *higher_priority_task_woken)
{
  /* TODO: Author Implementation */
  if(pipe && pipe->hw_regs && higher_priority_task_woken)
  {
    if((pipe->hw_regs->status & SENSOR_STATUS_DRDY) == SENSOR_STATUS_DRDY)
    {
      spsc_ring_push(&pipe->ring, pipe->hw_regs->data);
      pipe->hw_regs->status &= ~(SENSOR_STATUS_DRDY);

      /*
       * Interrupt mitigation: mask SENSOR_CR_IE in top-half to prevent IRQ storms
       * while bottom-half task is servicing the queue.
       * Bottom-half explicitly re-enables SENSOR_CR_IE once the sample is processed.
       */
      pipe->hw_regs->cr &= ~(SENSOR_CR_IE);

      *higher_priority_task_woken = true;
      rtos_sem_give_from_isr(&pipe->data_sem, higher_priority_task_woken);
    }
  }
}

/**
 * Bottom-half deferred processing function executed in RTOS task context.
 * Waits for a notification from the top-half ISR, dequeues the next sample,
 * updates the sliding window moving average, and computes the latest average.
 *
 * @param pipe Pointer to the sensor pipeline context structure.
 * @param timeout_ms Maximum time to wait for a sample in milliseconds (-1 for infinite).
 * @return 0 on successful sample processing, -1 on timeout, or negative error code.
 */
int sensor_process_next_sample(sensor_pipeline_t *pipe, int timeout_ms)
{
  /* TODO: Author Implementation */
  if((pipe == NULL) || (timeout_ms < -1))
  {
    return -1;
  }

  int ret = rtos_sem_wait(&pipe->data_sem, timeout_ms);

  if(ret == 0)
  {
    uint16_t sample = 0;
    if(spsc_ring_pop(&pipe->ring, &sample) == 0)
    {
      if(pipe->processed_count >= MOVING_AVG_WINDOW)
      {
        pipe->window_count -= pipe->window[pipe->window_idx];
        pipe->window[pipe->window_idx] = sample;
        pipe->window_count += pipe->window[pipe->window_idx];
        pipe->latest_avg = pipe->window_count / MOVING_AVG_WINDOW;
        pipe->processed_count++;
      }
      else
      {
        pipe->window[pipe->window_idx] = sample;
        pipe->window_count += sample;
        pipe->processed_count++;
        pipe->latest_avg = pipe->window_count / pipe->processed_count;
      }
      pipe->window_idx = (pipe->window_idx + 1) % MOVING_AVG_WINDOW;

      /*
       * Critical: Re-enable hardware interrupts now that the bottom-half
       * task has successfully retrieved and processed the sample from the ring.
       */
      if(pipe->hw_regs != NULL)
      {
        pipe->hw_regs->cr |= SENSOR_CR_IE;
      }

      return 0;
    }

    /* Queue was empty despite semaphore wake (spurious wake or race) */
    return -1;
  }
  else
  {
    return -1;
  }
}

/**
 * Destroys and cleans up resources allocated for the sensor pipeline.
 *
 * @param pipe Pointer to the sensor pipeline context structure.
 */
void sensor_pipeline_destroy(sensor_pipeline_t *pipe)
{
  if(pipe != NULL)
  {
    rtos_sem_destroy(&pipe->data_sem);
  }
}
