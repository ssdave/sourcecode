/**
 * @file spsc_ring.c
 * @brief Lock-Free SPSC Circular Ring Buffer & Mock RTOS Semaphore Primitives.
 */

#include "rtos_sensor.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

/* ==============================================================================
 * Mock RTOS Synchronization Primitives (FreeRTOS-Compatible Semantics)
 * ============================================================================== */

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
 * Single-Producer Single-Consumer (SPSC) Circular Ring Buffer
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
