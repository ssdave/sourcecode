/* ============================================================================
 * File: custom_semaphore.c
 *
 * Attribution & Copyright:
 *   Originates from Edsger W. Dijkstra (1965) "Cooperating Sequential Processes",
 *   POSIX.1-2001 semaphore semantics (IEEE Std 1003.1), and Remzi Arpaci-Dusseau
 *   & Andrea Arpaci-Dusseau (OSTEP Chapter 31: Semaphores).
 *   Completed independently for Systems & Concurrency self-study.
 *
 * Systems Engineering & Concurrency Architecture:
 *   1. Mesa Semantics vs. Hoare Semantics:
 *      POSIX condition variables follow Mesa semantics: signaling wakes up a
 *      waiting thread, but the signaler retains the mutex and the woken thread
 *      must re-contend for the lock. The semaphore condition predicate MUST be
 *      evaluated inside a `while` loop (never `if`) because:
 *        a) Spurious wakeups (hardware interrupts / OS signal delivery).
 *        b) Intervening threads: another thread may acquire the lock and decrement
 *           the semaphore before the woken thread re-acquires the mutex.
 *      Under Hoare semantics, signaling immediately transfers both the lock and
 *      CPU execution to the waiting thread with state invariants guaranteed.
 *
 *   2. Semaphore vs. Mutex Architectural Contracts:
 *      - Ownership: A mutex has strict thread ownership (the thread that locks
 *        MUST be the thread that unlocks). Semaphores have NO concept of ownership;
 *        one thread (or interrupt handler) can signal a semaphore that another
 *        thread is waiting on.
 *      - Async-Signal-Safety: Standard POSIX mutexes (`pthread_mutex_lock`) are
 *        NOT async-signal-safe (calling inside a signal handler causes deadlock or
 *        undefined behavior if the signal interrupted the lock holder). Native
 *        POSIX semaphores (`sem_post`) ARE async-signal-safe and can be used in ISRs.
 *      - Priority Inversion: Mutexes support Priority Inheritance Protocols
 *        (`PTHREAD_PRIO_INHERIT`), where a lower-priority lock holder temporarily
 *        inherits the higher priority of a blocked thread. Semaphores cannot support
 *        automatic priority inheritance because the identity of the future signaling
 *        thread is unknown.
 *
 * What is being built:
 *   A custom user-space counting semaphore (`custom_sem_t`) implemented using
 *   POSIX synchronization primitives (`pthread_mutex_t` and `pthread_cond_t`).
 *
 * What is being tested:
 *   A high-contention stress harness with NUM_WORKERS (12) threads competing for
 *   a bounded pool of capacity POOL_CAPACITY (3) across multiple iterations.
 *   Verifies mutual exclusion boundaries, spurious wakeup resilience, and zero
 *   lost updates under heavy concurrent post/wait operations.
 * ============================================================================ */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define POOL_CAPACITY 3
#define NUM_WORKERS 12
#define ITERATIONS_PER_WORKER 20

typedef struct
{
  int value;
  pthread_mutex_t lock;
  pthread_cond_t cond;
} custom_sem_t;

/**
 * Initializes the custom counting semaphore.
 *
 * @param sem Pointer to custom semaphore structure.
 * @param value Initial semaphore count (must be >= 0).
 * @return 0 on success, or -1 on invalid argument or initialization error.
 */
int custom_sem_init(custom_sem_t *sem, int value)
{
  /* TODO: Author Implementation */
  if((sem == NULL) || (value < 0))
  {
    printf("Incorrect arguments passed\n");
    return -1;
  }

  if(pthread_mutex_init(&sem->lock, NULL) != 0)
  {
    printf("Mutex init failed\n");
    return -1;
  }

  if(pthread_cond_init(&sem->cond, NULL) != 0)
  {
    printf("CV init failed\n");
    return -1;
  }
  sem->value = value;

  return 0;
}

/**
 * Decrements (waits / locks) the custom semaphore.
 *
 * @param sem Pointer to custom semaphore structure.
 * @return 0 on success, or -1 on error.
 */
int custom_sem_wait(custom_sem_t *sem)
{
  /* TODO: Author Implementation */

  if(sem == NULL)
  {
    return -1;
  }

  pthread_mutex_lock(&sem->lock);

  while(sem->value <= 0)
  {
    pthread_cond_wait(&sem->cond, &sem->lock);
  }

  sem->value--;

  pthread_mutex_unlock(&sem->lock);

  return 0;
}

/**
 * Increments (posts / signals) the custom semaphore.
 *
 * @param sem Pointer to custom semaphore structure.
 * @return 0 on success, or -1 on error.
 */
int custom_sem_post(custom_sem_t *sem)
{
  /* TODO: Author Implementation */
  if(sem == NULL)
  {
    return -1;
  }

  pthread_mutex_lock(&sem->lock);

  sem->value++;
  pthread_cond_signal(&sem->cond);

  pthread_mutex_unlock(&sem->lock);

  return 0;
}

/**
 * Destroys the custom semaphore and releases internal synchronization resources.
 *
 * @param sem Pointer to custom semaphore structure.
 * @return 0 on success, or -1 on error.
 */
int custom_sem_destroy(custom_sem_t *sem)
{
  if(sem == NULL)
  {
    return -1;
  }

  int ret = 0;

  if(pthread_mutex_destroy(&sem->lock) != 0)
  {
    printf("mutex destroy failed\n");
    ret = -1;
  }

  if(pthread_cond_destroy(&sem->cond) != 0)
  {
    printf("CV destroy failed\n");
    ret = -1;
  }

  sem->value = -1;

  return ret;
}

/* ============================================================================
 * High-Contention Stress Harness & Verification Engine
 * ============================================================================ */

static custom_sem_t pool_sem;
static pthread_mutex_t audit_lock = PTHREAD_MUTEX_INITIALIZER;
static int active_count = 0;
static int max_observed_active = 0;
static int total_entries = 0;
static int safety_invariant_broken = 0;

void *high_contention_worker(void *arg)
{
  long id = (long)arg;

  for(int i = 0; i < ITERATIONS_PER_WORKER; i++)
  {
    /* Randomized arrival jitter */
    usleep((useconds_t)((rand() % 500) + 100));

    if(custom_sem_wait(&pool_sem) != 0)
    {
      fprintf(stderr, "Worker %ld: custom_sem_wait failed\n", id);
      safety_invariant_broken = 1;
      return NULL;
    }

    /* Critical Section: Bounded Resource Pool */
    pthread_mutex_lock(&audit_lock);
    active_count++;
    if(active_count > max_observed_active)
    {
      max_observed_active = active_count;
    }
    if(active_count > POOL_CAPACITY)
    {
      safety_invariant_broken = 1;
    }
    total_entries++;
    int current_active = active_count;
    pthread_mutex_unlock(&audit_lock);

    /* Simulated bounded work */
    usleep((useconds_t)((rand() % 1000) + 200));

    pthread_mutex_lock(&audit_lock);
    active_count--;
    pthread_mutex_unlock(&audit_lock);

    if(custom_sem_post(&pool_sem) != 0)
    {
      fprintf(stderr, "Worker %ld: custom_sem_post failed\n", id);
      safety_invariant_broken = 1;
      return NULL;
    }

    (void)current_active;
  }

  return NULL;
}

int main(void)
{
  printf("=== Custom Counting Semaphore Stress Test ===\n");
  printf("Config: %d workers, %d capacity, %d iterations/worker (%d total operations)\n",
         NUM_WORKERS, POOL_CAPACITY, ITERATIONS_PER_WORKER, NUM_WORKERS * ITERATIONS_PER_WORKER);

  if(custom_sem_init(&pool_sem, POOL_CAPACITY) != 0)
  {
    printf("[Exercise Scaffold] Awaiting author implementation for custom_semaphore.\n");
    return 0;
  }

  pthread_t threads[NUM_WORKERS];
  for(long i = 0; i < NUM_WORKERS; i++)
  {
    pthread_create(&threads[i], NULL, high_contention_worker, (void *)i);
  }

  for(int i = 0; i < NUM_WORKERS; i++)
  {
    pthread_join(threads[i], NULL);
  }

  custom_sem_destroy(&pool_sem);

  int expected_entries = NUM_WORKERS * ITERATIONS_PER_WORKER;
  printf("Stress Summary: total_entries=%d, max_active=%d (capacity=%d)\n", total_entries,
         max_observed_active, POOL_CAPACITY);

  if(!safety_invariant_broken && total_entries == expected_entries &&
     max_observed_active <= POOL_CAPACITY)
  {
    printf("[PASS] custom_semaphore passed high-contention stress test with zero invariant "
           "violations.\n");
    return 0;
  }
  else
  {
    printf(
        "[FAIL] custom_semaphore invariant violated (safety_broken=%d, total=%d, max_active=%d).\n",
        safety_invariant_broken, total_entries, max_observed_active);
    return 1;
  }
}
