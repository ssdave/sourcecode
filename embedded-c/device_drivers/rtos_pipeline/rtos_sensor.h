/**
 * @file rtos_sensor.h
 * @brief Top-Half (ISR) vs. Bottom-Half (RTOS Task) Deferred Sensor Processing Interface.
 *
 * Concepts Demonstrated:
 * 1. Top-Half (ISR) vs. Bottom-Half (RTOS Task) deferred interrupt processing architecture.
 * 2. Hardware MMIO sensor register simulation (STATUS, DATA, CR).
 * 3. Lock-free Single-Producer Single-Consumer (SPSC) circular queue between ISR and Task.
 * 4. RTOS Semaphore synchronization (giving from ISR, blocking wait with timeout in task).
 * 5. Top-Half ISR handler: validates DRDY flag, reads MMIO DATA, acknowledges/clears IRQ,
 *    enqueues sample non-blockingly, and notifies RTOS task via semaphore.
 * 6. Bottom-Half task processing: blocks on semaphore, dequeues sample from SPSC queue,
 *    and maintains a sliding-window moving-average filter.
 *
 * Academic Attribution:
 * Exercise design based on real-time operating systems concepts and deferred interrupt
 * handling architectures (FreeRTOS Architecture, CS:APP Chapter 8, OSTEP Chapters 30-31,
 * and Linux Device Drivers Top-Half/Bottom-Half model).
 * Retained strictly for independent educational self-study.
 */

#ifndef RTOS_SENSOR_H
#define RTOS_SENSOR_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

int rtos_sem_init(rtos_sem_t *sem, int initial_count);
void rtos_sem_destroy(rtos_sem_t *sem);
int rtos_sem_wait(rtos_sem_t *sem, int timeout_ms);
void rtos_sem_give_from_isr(rtos_sem_t *sem, bool *higher_priority_task_woken);

/* ==============================================================================
 * SPSC Ring Buffer & Sensor Pipeline Context
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
 * SPSC Ring Buffer Function Declarations
 * ============================================================================== */

int spsc_ring_init(spsc_ring_t *ring);
int spsc_ring_push(spsc_ring_t *ring, uint16_t sample);
int spsc_ring_pop(spsc_ring_t *ring, uint16_t *sample);

/* ==============================================================================
 * Sensor Pipeline Function Declarations
 * ============================================================================== */

int sensor_pipeline_init(sensor_pipeline_t *pipe, sensor_mmio_regs_t *hw_regs);
void sensor_drdy_isr(sensor_pipeline_t *pipe, bool *higher_priority_task_woken);
int sensor_process_next_sample(sensor_pipeline_t *pipe, int timeout_ms);
void sensor_pipeline_destroy(sensor_pipeline_t *pipe);

#endif /* RTOS_SENSOR_H */
