/* ============================================================================
 * File: reader_writer_lock.c
 *
 * Attribution & Copyright:
 *   Originates from P.J. Courtois, F. Heymans, and D.L. Parnas (1971)
 *   "Concurrent Control with 'Readers' and 'Writers'", Communications of the ACM,
 *   POSIX.1-2001 (pthread_rwlock_t semantics), and Remzi Arpaci-Dusseau &
 *   Andrea Arpaci-Dusseau (OSTEP Chapter 31: Semaphores & Readers-Writers).
 *   Completed independently for Systems & Concurrency self-study.
 *
 * Systems Engineering & Concurrency Architecture:
 *   1. Reader Preference vs. Writer Preference vs. Fair (FIFO) Policies:
 *      - Reader Preference (Naive): Readers acquire the lock as long as no writer
 *        is actively writing. Flaw: Under high read throughput (e.g. cache queries),
 *        writers suffer unbounded starvation, leading to severe tail latency and
 *        stale data replication issues.
 *      - Writer Preference (Implemented Here): When a writer requests the lock,
 *        it increments `writers_waiting`. Incoming readers are immediately BLOCKED
 *        from entering the critical section, even if current readers are still
 *        active. Once active readers drain to zero, the waiting writer executes.
 *        This bounds writer latency and prevents starvation.
 *      - Fair / FIFO (Ticket/Queue-based): Requests are granted in strict arrival
 *        order (phase-fair or ticket-based rwlock), preventing both reader and
 *        writer starvation at the expense of slightly higher lock overhead.
 *
 *   2. Formal State Invariants:
 *      - Safety Invariant 1: `writer_active <= 1` (mutual exclusion among writers).
 *      - Safety Invariant 2: `(writer_active == 1) => (readers == 0)` (exclusive access).
 *      - Safety Invariant 3: `(readers > 0) => (writer_active == 0)` (shared read access).
 *      - Liveness Invariant: If `writers_waiting > 0`, incoming readers must wait,
 *        guaranteeing the lock drains for waiting writers.
 *
 * What is being built:
 *   A custom user-space Reader-Writer Lock (`rwlock_t`) with writer preference
 *   implemented using POSIX synchronization primitives (`pthread_mutex_t` and
 *   `pthread_cond_t`).
 *
 * What is being tested:
 *   An adversarial starvation test harness: 8 reader threads flood the lock with
 *   continuous read requests, while 3 writer threads issue updates. Verifies
 *   that waiting writers successfully preempt incoming readers without starving,
 *   and ensures zero data races or invariant violations occur.
 * ============================================================================ */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_READERS 8
#define NUM_WRITERS 3
#define WRITER_ITERATIONS 5
#define READER_ITERATIONS 20

typedef struct
{
  int readers;
  int writers_waiting;
  int writer_active;
  pthread_mutex_t lock;
  pthread_cond_t ok_to_read;
  pthread_cond_t ok_to_write;
} rwlock_t;

/**
 * Initializes the reader-writer lock with writer preference.
 *
 * @param rw Pointer to rwlock structure.
 * @return 0 on success, or -1 on error.
 */
int rwlock_init(rwlock_t *rw)
{
  /* TODO: Author Implementation */
  if(rw == NULL)
  {
    return -1;
  }

  rw->readers = 0;
  rw->writer_active = 0;
  rw->writers_waiting = 0;

  if(pthread_mutex_init(&rw->lock, NULL) != 0)
  {
    printf("Mutex init failed\n");
    return -1;
  }

  if(pthread_cond_init(&rw->ok_to_read, NULL) != 0)
  {
    printf("CV ok_to_read init failed\n");
    return -1;
  }

  if(pthread_cond_init(&rw->ok_to_write, NULL) != 0)
  {
    printf("CV ok_to_write init failed\n");
    return -1;
  }

  return 0;
}

/**
 * Acquires the reader-writer lock in shared read mode.
 * Blocks if a writer holds the lock or if writers are waiting (writer preference).
 *
 * @param rw Pointer to rwlock structure.
 * @return 0 on success, or -1 on error.
 */
int rwlock_read_lock(rwlock_t *rw)
{
  /* TODO: Author Implementation */
  if(rw == NULL)
  {
    return -1;
  }

  pthread_mutex_lock(&rw->lock);

  while((rw->writer_active > 0) || (rw->writers_waiting > 0))
  {
    pthread_cond_wait(&rw->ok_to_read, &rw->lock);
  }

  rw->readers++;

  pthread_mutex_unlock(&rw->lock);

  return 0;
}

/**
 * Releases the reader-writer lock from shared read mode.
 *
 * @param rw Pointer to rwlock structure.
 * @return 0 on success, or -1 on error.
 */
int rwlock_read_unlock(rwlock_t *rw)
{
  /* TODO: Author Implementation */
  if(rw == NULL)
  {
    return -1;
  }

  pthread_mutex_lock(&rw->lock);

  rw->readers--;

  if((rw->readers == 0) && (rw->writers_waiting > 0))
  {
    pthread_cond_signal(&rw->ok_to_write);
  }

  pthread_mutex_unlock(&rw->lock);

  return 0;
}

/**
 * Acquires the reader-writer lock in exclusive write mode.
 * Blocks until all active readers and writers have released the lock.
 *
 * @param rw Pointer to rwlock structure.
 * @return 0 on success, or -1 on error.
 */
int rwlock_write_lock(rwlock_t *rw)
{
  /* TODO: Author Implementation */
  if(rw == NULL)
  {
    return -1;
  }

  pthread_mutex_lock(&rw->lock);

  rw->writers_waiting++;

  while((rw->writer_active > 0) || (rw->readers > 0))
  {
    pthread_cond_wait(&rw->ok_to_write, &rw->lock);
  }

  rw->writers_waiting--;
  rw->writer_active++;

  pthread_mutex_unlock(&rw->lock);

  return 0;
}

/**
 * Releases the reader-writer lock from exclusive write mode.
 *
 * @param rw Pointer to rwlock structure.
 * @return 0 on success, or -1 on error.
 */
int rwlock_write_unlock(rwlock_t *rw)
{
  /* TODO: Author Implementation */

  if(rw == NULL)
  {
    return -1;
  }

  pthread_mutex_lock(&rw->lock);

  rw->writer_active--;

  if((rw->writer_active == 0) && (rw->writers_waiting > 0))
  {
    // Wake the next writer
    pthread_cond_signal(&rw->ok_to_write);
  }
  else if((rw->writer_active == 0) && (rw->writers_waiting == 0))
  {
    // Wake ALL waiting readers
    pthread_cond_broadcast(&rw->ok_to_read);
  }

  pthread_mutex_unlock(&rw->lock);

  return 0;
}

/**
 * Destroys the reader-writer lock and releases internal synchronization resources.
 *
 * @param rw Pointer to rwlock structure.
 * @return 0 on success, or -1 on error.
 */
int rwlock_destroy(rwlock_t *rw)
{
  /* TODO: Author Implementation */

  if(rw == NULL)
  {
    return -1;
  }

  int ret = 0;

  if(pthread_mutex_destroy(&rw->lock) != 0)
  {
    printf("mutex destroy failed\n");
    ret = -1;
  }

  if(pthread_cond_destroy(&rw->ok_to_write) != 0)
  {
    printf("CV ok_to_write destroy failed\n");
    ret = -1;
  }

  if(pthread_cond_destroy(&rw->ok_to_read) != 0)
  {
    printf("CV ok_to_read destroy failed\n");
    ret = -1;
  }

  rw->readers = -1;
  rw->writers_waiting = -1;
  rw->writer_active = -1;

  return ret;
}

/* ============================================================================
 * Adversarial Starvation Test Harness & Verification Engine
 * ============================================================================ */

static rwlock_t rw_lock;
static int shared_resource_val = 0;
static int active_readers_audit = 0;
static int active_writers_audit = 0;
static pthread_mutex_t audit_mutex = PTHREAD_MUTEX_INITIALIZER;
static int safety_violation_flag = 0;
static int total_reads_completed = 0;
static int total_writes_completed = 0;

void *adversarial_reader(void *arg)
{
  long id = (long)arg;

  for(int i = 0; i < READER_ITERATIONS; i++)
  {
    if(rwlock_read_lock(&rw_lock) != 0)
    {
      fprintf(stderr, "Reader %ld: rwlock_read_lock failed\n", id);
      safety_violation_flag = 1;
      return NULL;
    }

    /* Verification of Safety Invariants */
    pthread_mutex_lock(&audit_mutex);
    active_readers_audit++;
    if(active_writers_audit > 0)
    {
      fprintf(stderr, "[FAIL] Reader %ld active while writer is writing!\n", id);
      safety_violation_flag = 1;
    }
    int val = shared_resource_val;
    total_reads_completed++;
    pthread_mutex_unlock(&audit_mutex);

    /* Simulated read work */
    usleep((useconds_t)((rand() % 300) + 100));

    pthread_mutex_lock(&audit_mutex);
    active_readers_audit--;
    pthread_mutex_unlock(&audit_mutex);

    if(rwlock_read_unlock(&rw_lock) != 0)
    {
      fprintf(stderr, "Reader %ld: rwlock_read_unlock failed\n", id);
      safety_violation_flag = 1;
      return NULL;
    }

    usleep((useconds_t)((rand() % 200) + 50));
    (void)val;
  }

  return NULL;
}

void *adversarial_writer(void *arg)
{
  long id = (long)arg;

  for(int i = 0; i < WRITER_ITERATIONS; i++)
  {
    /* Small inter-write delay */
    usleep((useconds_t)((rand() % 1000) + 500));

    if(rwlock_write_lock(&rw_lock) != 0)
    {
      fprintf(stderr, "Writer %ld: rwlock_write_lock failed\n", id);
      safety_violation_flag = 1;
      return NULL;
    }

    /* Verification of Safety Invariants */
    pthread_mutex_lock(&audit_mutex);
    active_writers_audit++;
    if(active_writers_audit > 1 || active_readers_audit > 0)
    {
      fprintf(stderr, "[FAIL] Writer %ld active while others in CS (writers=%d, readers=%d)!\n", id,
              active_writers_audit, active_readers_audit);
      safety_violation_flag = 1;
    }
    shared_resource_val += 10;
    total_writes_completed++;
    pthread_mutex_unlock(&audit_mutex);

    /* Simulated write work */
    usleep((useconds_t)((rand() % 500) + 200));

    pthread_mutex_lock(&audit_mutex);
    active_writers_audit--;
    pthread_mutex_unlock(&audit_mutex);

    if(rwlock_write_unlock(&rw_lock) != 0)
    {
      fprintf(stderr, "Writer %ld: rwlock_write_unlock failed\n", id);
      safety_violation_flag = 1;
      return NULL;
    }
  }

  return NULL;
}

int main(void)
{
  printf("=== Reader-Writer Lock Test (Writer Preference) ===\n");
  printf("Config: %d readers (%d reads each), %d writers (%d writes each)\n", NUM_READERS,
         READER_ITERATIONS, NUM_WRITERS, WRITER_ITERATIONS);

  if(rwlock_init(&rw_lock) != 0)
  {
    printf("[Exercise Scaffold] Awaiting author implementation for reader_writer_lock.\n");
    return 0;
  }

  pthread_t readers[NUM_READERS];
  pthread_t writers[NUM_WRITERS];

  for(long i = 0; i < NUM_READERS; i++)
  {
    pthread_create(&readers[i], NULL, adversarial_reader, (void *)i);
  }

  for(long i = 0; i < NUM_WRITERS; i++)
  {
    pthread_create(&writers[i], NULL, adversarial_writer, (void *)i);
  }

  for(int i = 0; i < NUM_READERS; i++)
  {
    pthread_join(readers[i], NULL);
  }

  for(int i = 0; i < NUM_WRITERS; i++)
  {
    pthread_join(writers[i], NULL);
  }

  rwlock_destroy(&rw_lock);

  int expected_final_val = NUM_WRITERS * WRITER_ITERATIONS * 10;
  printf("Summary: reads=%d, writes=%d, final_value=%d (expected=%d)\n", total_reads_completed,
         total_writes_completed, shared_resource_val, expected_final_val);

  if(!safety_violation_flag && shared_resource_val == expected_final_val)
  {
    printf("[PASS] reader_writer_lock passed adversarial starvation test with zero violations.\n");
    return 0;
  }
  else
  {
    printf("[FAIL] reader_writer_lock invariant or starvation violation detected.\n");
    return 1;
  }
}
