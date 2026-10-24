/**
 * @file rtos_sensor_pipeline.c
 * @brief Top-Half (ISR) vs. Bottom-Half (RTOS Task) Deferred Sensor Processing Pipeline.
 *
 * Concepts Demonstrated:
 * 1. Top-Half (ISR) vs. Bottom-Half (RTOS Task) deferred interrupt processing architecture.
 * 2. Hardware MMIO sensor register simulation:
 *    - STATUS register (DRDY data-ready and ERR fault flags).
 *    - DATA register (16-bit raw digital conversion sample).
 *    - CR control register (IE interrupt enable and START conversion flags).
 * 3. Lock-free Single-Producer Single-Consumer (SPSC) circular queue between ISR and Task:
 *    - Non-blocking push with dropped sample overflow accounting (an ISR never blocks!).
 *    - Task-side consumer pop.
 * 4. RTOS Semaphore synchronization:
 *    - Giving counting semaphore from ISR context with context-switch request flag.
 *    - Blocking wait with timeout in RTOS task context.
 * 5. Top-Half ISR handler: validates DRDY flag, reads MMIO DATA, acknowledges/clears IRQ,
 *    enqueues sample non-blockingly, and notifies RTOS task via semaphore.
 * 6. Bottom-Half task processing: blocks on semaphore, dequeues sample from SPSC queue,
 *    and maintains a sliding-window moving-average filter.
 *
 * RTOS Sensor Pipeline Architecture (Top-Half ISR / Bottom-Half Task):
 *
 *  +-------------------------------------------------------------------------+
 *  | HARDWARE / SENSOR PERIPHERAL (MMIO)                                      |
 *  |  - STATUS: DRDY (Data Ready), ERR (Fault Flag)                          |
 *  |  - DATA:   16-bit Raw Sensor Conversion (ADC / Gyro / Accel)            |
 *  |  - CR:     IE (Interrupt Enable), START (Continuous Conversion)         |
 *  +-------------------------------------------------------------------------+
 *                                     |
 *                                     | Hardware Interrupt (IRQ)
 *                                     v
 *  +-------------------------------------------------------------------------+
 *  | TOP-HALF: INTERRUPT SERVICE ROUTINE (sensor_drdy_isr)                   |
 *  |  - Hardware interrupt handling & acknowledgement                        |
 *  |  - Non-blocking sample push into circular buffer                        |
 *  |  - RTOS worker task notification                                        |
 *  +-------------------------------------------------------------------------+
 *                                     |
 *                 +-------------------+-------------------+
 *                 | Data Path                             | Control Path
 *                 v                                       v
 *  +-----------------------------+         +-----------------------------+
 *  | SPSC CIRCULAR RING BUFFER   |         | RTOS COUNTING SEMAPHORE     |
 *  | Lock-free SPSC Ring         |         | (Unblocks Worker Task)      |
 *  +-----------------------------+         +-----------------------------+
 *                 |                                       |
 *                 +-------------------+-------------------+
 *                                     |
 *                                     v
 *  +-------------------------------------------------------------------------+
 *  | BOTTOM-HALF: RTOS WORKER TASK (sensor_process_next_sample)              |
 *  |  - Deferred sensor sample processing                                    |
 *  |  - Sliding-window moving-average filter computation                     |
 *  +-------------------------------------------------------------------------+
 *
 * Academic Attribution:
 * Exercise design based on real-time operating systems concepts and deferred interrupt
 * handling architectures (FreeRTOS Architecture, CS:APP Chapter 8, OSTEP Chapters 30-31,
 * and Linux Device Drivers Top-Half/Bottom-Half model).
 * Retained strictly for independent educational self-study.
 */

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* ==============================================================================
 * Hardware Abstraction Layer (HAL) & Mock Sensor MMIO Registers
 * ============================================================================== */

#define SENSOR_STATUS_DRDY (1U << 0) /* Data Ready flag (set by hardware, cleared by ISR) */
#define SENSOR_STATUS_ERR (1U << 1)  /* Sensor fault / error flag */

#define SENSOR_CR_IE (1U << 0)    /* Interrupt Enable */
#define SENSOR_CR_START (1U << 1) /* Start continuous conversion */

/**
 * Memory-Mapped I/O Register Layout (Mock Sensor Peripheral)
 */
typedef struct
{
  volatile uint16_t status; /* Status register (DRDY, ERR) */
  volatile uint16_t data;   /* 16-bit raw sensor data (e.g., ADC reading) */
  volatile uint16_t cr;     /* Control register (IE, START) */
} sensor_mmio_regs_t;

/* ==============================================================================
 * Mock RTOS Synchronization Primitives (FreeRTOS-Compatible Semantics)
 * ============================================================================== */

/**
 * RTOS Counting Semaphore
 */
typedef struct
{
  pthread_mutex_t lock;
  pthread_cond_t cond;
  int count;
} rtos_sem_t;

/**
 * Initializes an RTOS counting semaphore.
 *
 * @param sem Pointer to the semaphore structure.
 * @param initial_count Initial token count (>= 0).
 * @return 0 on success, or -1 on invalid argument or system error.
 */
int rtos_sem_init(rtos_sem_t *sem, int initial_count)
{
  if(sem == NULL || initial_count < 0)
  {
    return -1;
  }
  if(pthread_mutex_init(&sem->lock, NULL) != 0)
  {
    return -1;
  }
  if(pthread_cond_init(&sem->cond, NULL) != 0)
  {
    pthread_mutex_destroy(&sem->lock);
    return -1;
  }
  sem->count = initial_count;
  return 0;
}

/**
 * Destroys an RTOS counting semaphore.
 *
 * @param sem Pointer to the semaphore structure.
 */
void rtos_sem_destroy(rtos_sem_t *sem)
{
  if(sem != NULL)
  {
    pthread_mutex_destroy(&sem->lock);
    pthread_cond_destroy(&sem->cond);
  }
}

/**
 * Waits on an RTOS counting semaphore with a timeout.
 *
 * @param sem Pointer to the semaphore structure.
 * @param timeout_ms Maximum time to wait in milliseconds (-1 for infinite wait, 0 for
 * non-blocking).
 * @return 0 if token acquired, -1 on timeout or failure.
 */
int rtos_sem_wait(rtos_sem_t *sem, int timeout_ms)
{
  if(sem == NULL)
  {
    return -1;
  }

  pthread_mutex_lock(&sem->lock);

  if(timeout_ms < 0)
  {
    /* Infinite blocking wait */
    while(sem->count == 0)
    {
      pthread_cond_wait(&sem->cond, &sem->lock);
    }
    sem->count--;
    pthread_mutex_unlock(&sem->lock);
    return 0;
  }

  if(timeout_ms == 0)
  {
    /* Non-blocking try-wait */
    if(sem->count > 0)
    {
      sem->count--;
      pthread_mutex_unlock(&sem->lock);
      return 0;
    }
    pthread_mutex_unlock(&sem->lock);
    return -1;
  }

  /* Timed wait */
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += timeout_ms / 1000;
  ts.tv_nsec += (long)(timeout_ms % 1000) * 1000000L;
  if(ts.tv_nsec >= 1000000000L)
  {
    ts.tv_sec += ts.tv_nsec / 1000000000L;
    ts.tv_nsec %= 1000000000L;
  }

  while(sem->count == 0)
  {
    int rc = pthread_cond_timedwait(&sem->cond, &sem->lock, &ts);
    if(rc != 0)
    {
      pthread_mutex_unlock(&sem->lock);
      return -1; /* Timeout */
    }
  }

  sem->count--;
  pthread_mutex_unlock(&sem->lock);
  return 0;
}

/**
 * Signals an RTOS counting semaphore from interrupt context (Top-Half ISR).
 *
 * @param sem Pointer to the semaphore structure.
 * @param higher_priority_task_woken Pointer to boolean flag set to true if a task was unblocked.
 */
void rtos_sem_give_from_isr(rtos_sem_t *sem, bool *higher_priority_task_woken)
{
  if(sem == NULL)
  {
    return;
  }

  pthread_mutex_lock(&sem->lock);
  sem->count++;
  pthread_cond_signal(&sem->cond);
  if(higher_priority_task_woken != NULL)
  {
    *higher_priority_task_woken = true;
  }
  pthread_mutex_unlock(&sem->lock);
}

/* ==============================================================================
 * Driver Data Structures & Context
 * ============================================================================== */

#define SPSC_RING_CAPACITY 8
#define MOVING_AVG_WINDOW 4

/**
 * Single-Producer Single-Consumer (SPSC) Circular Ring Buffer.
 */
typedef struct
{
  uint16_t buffer[SPSC_RING_CAPACITY];
  volatile uint32_t head;    /* Producer write index */
  volatile uint32_t tail;    /* Consumer read index */
  volatile uint32_t dropped; /* Overflow counter for dropped samples */
} spsc_ring_t;

/**
 * Sensor Pipeline Context Structure.
 */
typedef struct
{
  sensor_mmio_regs_t *hw_regs; /* Pointer to hardware MMIO register block */
  spsc_ring_t ring;            /* SPSC circular queue between ISR and task */
  rtos_sem_t data_sem;         /* Notification semaphore */

  /* Bottom-half moving-average filter state */
  uint16_t window[MOVING_AVG_WINDOW];
  uint32_t window_idx;
  uint32_t window_count;
  uint32_t processed_count;
  uint32_t latest_avg;
} sensor_pipeline_t;

/* ==============================================================================
 * Driver Function Declarations & Skeletons
 * ============================================================================== */

/**
 * Initializes the single-producer single-consumer (SPSC) circular ring buffer.
 *
 * @param ring Pointer to the SPSC ring buffer structure.
 * @return 0 on success, or -1 on invalid argument.
 */
int spsc_ring_init(spsc_ring_t *ring)
{
  /* TODO: Author Implementation */
  if(ring == NULL)
  {
    return -1;
  }

  ring->dropped = 0;
  ring->head = 0;
  ring->tail = 0;

  return 0;
}

/**
 * Pushes a 16-bit sensor sample into the SPSC circular ring buffer.
 * Executed in producer context (such as an interrupt handler).
 *
 * @param ring Pointer to the SPSC ring buffer structure.
 * @param sample 16-bit sensor data value to insert.
 * @return 0 if successfully pushed, or -1 if ring is full (sample dropped).
 */
int spsc_ring_push(spsc_ring_t *ring, uint16_t sample)
{
  /* TODO: Author Implementation */
  if(ring == NULL)
  {
    return -1;
  }

  if((ring->head - ring->tail) == SPSC_RING_CAPACITY)
  {
    // AI Review Feedback: Excellent use of cumulative increment (ring->dropped++)
    // rather than latching (= 1). In production telemetry and health monitoring,
    // tracking cumulative dropped sample counts allows diagnosing burst overloads
    // and ring sizing under heavy IRQ rates.
    ring->dropped++;
    return -1;
  }

  ring->buffer[ring->head & (SPSC_RING_CAPACITY - 1)] = sample;
  ring->head++;

  return 0;
}

/**
 * Pops a 16-bit sensor sample from the SPSC circular ring buffer.
 * Executed in consumer context (such as an RTOS worker task).
 *
 * @param ring Pointer to the SPSC ring buffer structure.
 * @param sample Pointer to destination buffer where popped sample will be stored.
 * @return 0 if successfully popped, or -1 if ring is empty.
 */
int spsc_ring_pop(spsc_ring_t *ring, uint16_t *sample)
{
  /* TODO: Author Implementation */
  if(ring == NULL)
  {
    return -1;
  }

  if((ring->head - ring->tail) == 0)
  {
    return -1;
  }

  *sample = ring->buffer[ring->tail & (SPSC_RING_CAPACITY - 1)];
  ring->tail++;

  return 0;
}

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

/* ==============================================================================
 * Verification Test Harness
 * ============================================================================== */

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
