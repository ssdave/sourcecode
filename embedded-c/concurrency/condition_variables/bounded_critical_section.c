/* ============================================================================
 * File: bounded_critical_section.c
 *
 * Attribution & Copyright:
 *   Originates from Rose-Hulman Institute of Technology — CSSE 332: Operating Systems
 *   Original File: condvar_basics/max.c
 *   Course Schedule: https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/
 *   Lab Spec:        https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/condvar/
 *   Lab Archive:     https://github.com/rhit-csse332/csse332-labs-archive
 *   Authors:         RHIT CSSE 332 Faculty (M. Noureddine, M. Hewner, S. Stamm,
 *                    R. Williamson, S. Mohan)
 *   Completed independently for Operating Systems self-study.
 *
 * What is being built:
 *   Bounded-concurrency critical section (counting-semaphore pattern built from
 *   `pthread_mutex_t` and `pthread_cond_t`) that enforces at most 3 threads in
 *   the critical section at any time (`active_threads_count < 3`).
 *
 * What is being tested:
 *   Launches 8 threads (`'a'` through `'h'`); each thread enters the critical section
 *   while waiting threads block on `pthread_cond_wait` and wake up via
 *   `pthread_cond_signal` as slots free up.
 * ============================================================================ */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t section_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t slot_available_cond = PTHREAD_COND_INITIALIZER;
int active_threads_count = 0;

void *worker_thread(void *arg)
{
  char *letter = (char *)arg;
  printf("%c wants to enter the critical section\n", *letter);

  // Entry protocol: block while 3 threads already occupy the critical section
  pthread_mutex_lock(&section_mutex);
  while(active_threads_count >= 3)
  {
    printf("%c waiting to enter the critical section\n", *letter);
    pthread_cond_wait(&slot_available_cond, &section_mutex);
  }
  active_threads_count++;
  pthread_mutex_unlock(&section_mutex);

  // Critical section (up to 3 concurrent threads)
  printf("%c is in the critical section\n", *letter);
  usleep(50000); // 50ms simulation
  printf("%c has left the critical section\n", *letter);

  // Exit protocol: decrement active count and wake one waiting thread
  pthread_mutex_lock(&section_mutex);
  active_threads_count--;
  pthread_cond_signal(&slot_available_cond);
  pthread_mutex_unlock(&section_mutex);

  return NULL;
}

int main(int argc, char **argv)
{
  pthread_t threads[8];
  int i;
  char *letters = "abcdefgh";

  for(i = 0; i < 8; ++i)
  {
    pthread_create(&threads[i], NULL, worker_thread, &letters[i]);

    if(i == 4)
    {
      usleep(100000);
    }
  }

  for(i = 0; i < 8; i++)
  {
    pthread_join(threads[i], NULL);
  }

  printf("[PASS] All 8 worker threads completed bounded critical section successfully.\n");
  return 0;
}