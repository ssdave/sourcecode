/* ============================================================================
 * File: basicMutex1.c
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
 *   to eliminate read-modify-write data races on a global integer (`total`).
 *
 * What is being tested:
 *   Spawns NUM_THREADS (10) concurrent threads, each incrementing `total` 10,000
 *   times inside a mutex-protected critical section, and verifies that the final
 *   accumulated total is deterministically 100,000 with zero lost updates.
 * ============================================================================ */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_THREADS 10
/*
 * You can use the provided makefile to compile your code.
 */
int total;

pthread_mutex_t lock;

void *add10000(void *arg)
{
  for(int i = 0; i < 10000; i++)
  {
    // this area: critical section
    // want this to run in mutual exclusion mode
    pthread_mutex_lock(&lock);
    total++;
    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

int main(int argc, char **argv)
{
  total = 0;
  pthread_t threads[NUM_THREADS];
  pthread_mutex_init(&lock, 0);

  int i;
  for(i = 0; i < NUM_THREADS; i++)
  {
    pthread_create(&threads[i], NULL, add10000, NULL);
  }

  for(i = 0; i < NUM_THREADS; i++)
  {
    pthread_join(threads[i], NULL);
  }

  printf("\nEverything finished.  Final total %d\n", total);
  pthread_mutex_destroy(&lock);

  return 0;
}