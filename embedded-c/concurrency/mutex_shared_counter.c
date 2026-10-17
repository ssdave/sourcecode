/* ============================================================================
 * File: mutex_shared_counter.c
 *
 * Attribution & Copyright:
 *   Originates from Rose-Hulman Institute of Technology — CSSE 332: Operating Systems
 *   Original File: threads_basics/basic_mutex/add_a_lot.c
 *   Course Schedule: https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/
 *   Lab Spec:        https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/threads/
 *   Lab Archive:     https://github.com/rhit-csse332/csse332-labs-archive
 *   Authors:         RHIT CSSE 332 Faculty (M. Noureddine, M. Hewner, S. Stamm,
 *                    R. Williamson, S. Mohan)
 *   Completed independently for Operating Systems self-study.
 *
 * What is being built:
 *   Thread-safe shared counter accumulation using a POSIX mutex (pthread_mutex_t)
 *   to eliminate read-modify-write data races on a global integer (`counter_total`).
 *
 * What is being tested:
 *   Spawns NUM_THREADS (10) concurrent threads, each incrementing `counter_total`
 *   10,000 times inside a mutex-protected critical section, and verifies that the
 *   final accumulated total is deterministically 100,000 with zero lost updates.
 * ============================================================================ */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_THREADS 10

int counter_total;
pthread_mutex_t counter_mutex;

void *worker_increment_counter(void *arg)
{
  for(int i = 0; i < 10000; i++)
  {
    // Mutual exclusion: lock protects read-modify-write on shared counter
    pthread_mutex_lock(&counter_mutex);
    counter_total++;
    pthread_mutex_unlock(&counter_mutex);
  }

  return NULL;
}

int main(int argc, char **argv)
{
  counter_total = 0;
  pthread_t threads[NUM_THREADS];
  pthread_mutex_init(&counter_mutex, 0);

  int i;
  for(i = 0; i < NUM_THREADS; i++)
  {
    pthread_create(&threads[i], NULL, worker_increment_counter, NULL);
  }

  for(i = 0; i < NUM_THREADS; i++)
  {
    pthread_join(threads[i], NULL);
  }

  printf("\nEverything finished. Final total %d\n", counter_total);
  if(counter_total == 100000)
  {
    printf("[PASS] Final total is exactly 100,000\n");
  }
  else
  {
    printf("[FAIL] Expected 100,000, got %d\n", counter_total);
  }
  pthread_mutex_destroy(&counter_mutex);

  return 0;
}