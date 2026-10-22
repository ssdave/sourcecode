/* ============================================================================
 * File: atomic_spinlock.c
 *
 * Attribution & Copyright:
 *   Originates from Maurice Herlihy and Nir Shavit (2008) "The Art of
 *   Multiprocessor Programming", ISO/IEC 9899:2011 (C11 stdatomic.h), and
 *   Remzi Arpaci-Dusseau & Andrea Arpaci-Dusseau (OSTEP Chapter 28: Locks).
 *   Completed independently for Systems & Concurrency self-study.
 *
 * Systems Engineering & Concurrency Architecture:
 *   1. Memory Ordering Semantics (C11 Atomics):
 *      - `memory_order_acquire`: Enforces a one-way memory barrier. No memory
 *        reads or writes in the current thread can be reordered BEFORE this
 *        operation. Used when acquiring the lock to ensure operations inside the
 *        critical section do not execute prematurely.
 *      - `memory_order_release`: Enforces that all prior memory reads and writes
 *        in the current thread are committed and visible to other threads BEFORE
 *        this store operation takes effect. Used when releasing the lock.
 *      - `memory_order_seq_cst`: Full sequential consistency (default). Emits
 *        strictest hardware barriers, enforcing a single total order across all
 *        threads at the cost of higher instruction latency.
 *
 *   2. Hardware CPU Memory Barriers vs. Compiler Barriers:
 *      - Compiler Barrier (`asm volatile("" ::: "memory")`): Informs the compiler
 *        not to hoist or sink memory accesses across the barrier during optimization;
 *        however, it emits NO hardware CPU pipeline instructions.
 *      - CPU Hardware Barrier (e.g. `dmb ish` on ARM, `lock` prefix or `mfence` on x86):
 *        Modern out-of-order execution cores and store buffers reorder memory
 *        transactions at runtime. Hardware barriers enforce order across CPU cores.
 *
 *   3. Cache Coherency, Cache-Line Bouncing & False Sharing:
 *      - The MESI Protocol: Cores share cache lines in Modified (M), Exclusive (E),
 *        Shared (S), or Invalid (I) states.
 *      - Cache Line Bouncing: If threads spin using atomic Read-Modify-Write (RMW)
 *        instructions (`test_and_set`), each attempt demands exclusive write
 *        ownership (M state), invalidating the line across all other CPU core caches.
 *        The cache line violently bounces between cores over the interconnect,
 *        saturating the bus and destroying throughput.
 *      - Backoff & CPU Pause Hint: Under heavy spinning, inserting a pause hint
 *        (e.g., `sched_yield()` or CPU pause instruction) prevents speculative
 *        execution pipeline stalls and reduces power consumption.
 *      - False Sharing: If two independent variables reside within the same 64-byte
 *        cache line, modifying one forces invalidation of the other on other cores.
 *
 * What is being built:
 *   A user-space spinlock primitive (`spinlock_t`) built using C11 atomic flag
 *   operations (`atomic_flag`), providing busy-wait mutual exclusion with explicit
 *   acquire/release memory ordering.
 *
 * What is being tested:
 *   A high-contention stress test: NUM_THREADS (8) concurrent threads executing
 *   100,000 increments each on a shared counter protected by the spinlock.
 *   Verifies that the final count is deterministically 800,000 with zero lost
 *   updates and zero data races.
 * ============================================================================ */

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_SPIN_THREADS 8
#define INCREMENTS_PER_THREAD 100000

typedef struct
{
  atomic_flag flag;
} spinlock_t;

/* Set to 1 once author implementation is complete */
static int is_implemented = 1;

/**
 * Initializes the spinlock to an unlocked state.
 *
 * @param lock Pointer to spinlock structure.
 */
void spinlock_init(spinlock_t *lock)
{
  /* TODO: Author Implementation */
  if(lock == NULL)
  {
    return;
  }
  atomic_flag_clear(&lock->flag);
}

/**
 * Acquires the spinlock, busy-waiting (spinning) until the lock is acquired.
 *
 * @param lock Pointer to spinlock structure.
 */
void spinlock_lock(spinlock_t *lock)
{
  /* TODO: Author Implementation */
  if(lock == NULL)
  {
    return;
  }
  while(atomic_flag_test_and_set_explicit(&lock->flag, memory_order_acquire))
  {
  }
}

/**
 * Releases the spinlock.
 *
 * @param lock Pointer to spinlock structure.
 */
void spinlock_unlock(spinlock_t *lock)
{
  /* TODO: Author Implementation */
  if(lock == NULL)
  {
    return;
  }
  atomic_flag_clear_explicit(&lock->flag, memory_order_release);
}

/* ============================================================================
 * High-Contention Stress Test Harness & Verification Engine
 * ============================================================================ */

static spinlock_t counter_spinlock;
static long long shared_counter = 0;

void *spin_worker_thread(void *arg)
{
  (void)arg;

  for(int i = 0; i < INCREMENTS_PER_THREAD; i++)
  {
    spinlock_lock(&counter_spinlock);
    shared_counter++;
    spinlock_unlock(&counter_spinlock);
  }

  return NULL;
}

int main(void)
{
  printf("=== User-Space Atomic Spinlock Stress Test ===\n");
  long long expected_count = (long long)NUM_SPIN_THREADS * INCREMENTS_PER_THREAD;
  printf("Config: %d threads, %d increments/thread (%lld total increments)\n", NUM_SPIN_THREADS,
         INCREMENTS_PER_THREAD, expected_count);

  if(!is_implemented)
  {
    printf("[Exercise Scaffold] Awaiting author implementation for atomic_spinlock.\n");
    return 0;
  }

  spinlock_init(&counter_spinlock);
  shared_counter = 0;

  pthread_t threads[NUM_SPIN_THREADS];
  for(long i = 0; i < NUM_SPIN_THREADS; i++)
  {
    pthread_create(&threads[i], NULL, spin_worker_thread, (void *)i);
  }

  for(int i = 0; i < NUM_SPIN_THREADS; i++)
  {
    pthread_join(threads[i], NULL);
  }

  printf("Final counter: %lld (expected: %lld)\n", shared_counter, expected_count);

  if(shared_counter == expected_count)
  {
    printf(
        "[PASS] atomic_spinlock successfully verified at %lld increments with zero lost updates.\n",
        expected_count);
    return 0;
  }
  else
  {
    printf("[FAIL] atomic_spinlock lost updates detected (got %lld, expected %lld).\n",
           shared_counter, expected_count);
    return 1;
  }
}
