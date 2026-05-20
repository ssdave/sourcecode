/* ============================================================================
 * File: barrier.c
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
 *   Spawns NUM_THREADS (5) threads with randomized pre-barrier sleep durations
 *   (`rand() % 10`) and staggered creation; verifies that no thread prints
 *   "left the barrier" until all 5 threads have printed "waiting at barrier".
 * ============================================================================ */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <pthread.h>

/**
 * In this exercise, we would like to implement a barrier to make all of our
 * threads wait at a certain location in the code before they can move formward.
 * No thread can get passed the barrier until all threads reach that barrier,
 * then they can move out together in any order.
 */

#define NUM_THREADS 5

volatile int num_threads_at_barrier = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

void barrier_wait(void)
{
  pthread_mutex_lock(&lock);
  num_threads_at_barrier++;
  // The Nth (final) arriving thread broadcasts to wake all N-1 waiting threads
  if(num_threads_at_barrier == NUM_THREADS)
  {
    pthread_cond_broadcast(&cv);
  }
  else
  {
    // Earlier arrivals sleep on cv until the final thread reaches the barrier
    while(num_threads_at_barrier != NUM_THREADS)
    {
      pthread_cond_wait(&cv, &lock);
    }
  }
  pthread_mutex_unlock(&lock);
}

void *thread_fn(void *arg)
{
  int tid = *(int *)arg;

  printf("Thread %d started ...\n", tid);

  sleep(rand() % 10);

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
      sleep(2);
  }

  for(i = 0; i < NUM_THREADS; i++)
  {
    pthread_join(threads[i], NULL);
  }

  printf("All done, goodbye...\n");
  exit(0);
}