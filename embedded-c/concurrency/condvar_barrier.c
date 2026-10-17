/* ============================================================================
 * File: condvar_barrier.c
 *
 * Attribution & Copyright:
 *   Originates from Rose-Hulman Institute of Technology — CSSE 332: Operating Systems
 *   Original File: activities/condvar_intro/barrier.c
 *   Course Schedule: https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/
 *   Session Notes:   https://www.rose-hulman.edu/class/csse/csse332/2324b/notes/session19/
 *   Lab Archive:     https://github.com/rhit-csse332/csse332-labs-archive
 *   Authors:         RHIT CSSE 332 Faculty (M. Noureddine, M. Hewner, S. Stamm,
 *                    R. Williamson, S. Mohan)
 *   Completed independently for Operating Systems self-study.
 *
 * What is being built:
 *   An N-thread synchronization barrier (`barrier_wait`) using a mutex,
 *   shared arrival counter (`num_threads_at_barrier`), and condition variable
 *   broadcast (`pthread_cond_broadcast`).
 *
 * What is being tested:
 *   Spawns NUM_THREADS (5) threads with randomized pre-barrier delays; verifies
 *   that no thread leaves the barrier until all 5 threads have arrived.
 * ============================================================================ */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <pthread.h>

#define NUM_THREADS 5

volatile int num_threads_at_barrier = 0;
pthread_mutex_t barrier_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t barrier_cond = PTHREAD_COND_INITIALIZER;

void barrier_wait(void)
{
  pthread_mutex_lock(&barrier_mutex);
  num_threads_at_barrier++;
  // The Nth (final) arriving thread broadcasts to wake all N-1 waiting threads
  if(num_threads_at_barrier == NUM_THREADS)
  {
    pthread_cond_broadcast(&barrier_cond);
  }
  else
  {
    // Earlier arrivals sleep on barrier_cond until the final thread reaches the barrier
    while(num_threads_at_barrier != NUM_THREADS)
    {
      pthread_cond_wait(&barrier_cond, &barrier_mutex);
    }
  }
  pthread_mutex_unlock(&barrier_mutex);
}

void *thread_fn(void *arg)
{
  int tid = *(int *)arg;

  printf("Thread %d started ...\n", tid);

  usleep((rand() % 10) * 10000); // 0-90ms randomized arrival delay

  printf("Thread %d waiting at barrier ...\n", tid);
  barrier_wait();
  printf("Thread %d left the barrier ...\n", tid);

  return NULL;
}

int main()
{
  pthread_t threads[NUM_THREADS];
  int tids[NUM_THREADS];
  int i;

  srand(time(NULL));

  for(i = 0; i < NUM_THREADS; i++)
  {
    tids[i] = i;
    pthread_create(&threads[i], NULL, thread_fn, &tids[i]);

    if(i == NUM_THREADS / 2)
    {
      usleep(50000);
    }
  }

  for(i = 0; i < NUM_THREADS; i++)
  {
    pthread_join(threads[i], NULL);
  }

  printf("[PASS] All %d threads completed barrier synchronization successfully.\n", NUM_THREADS);
  return 0;
}