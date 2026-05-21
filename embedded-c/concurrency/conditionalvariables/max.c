/* ============================================================================
 * File: max.c
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
 *   the critical section at any time (`numInCriticalSection < 3`).
 *
 * What is being tested:
 *   Launches 8 threads (`'a'` through `'h'`) with a 4-second pause after the 5th
 *   thread; each thread sleeps 1 second inside the critical section while waiting
 *   threads block on `pthread_cond_wait` and wake up via `pthread_cond_signal`
 *   as slots free up.
 * ============================================================================ */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
int numInCriticalSection = 0;

void *thread(void *arg)
{
  char *letter = (char *)arg;
  printf("%c wants to enter the critical section\n", *letter);

  // Entry protocol: block while 3 threads already occupy the critical section
  pthread_mutex_lock(&lock);
  while(numInCriticalSection >= 3)
  {
    printf("%c waiting to enter the critical section\n", *letter);
    pthread_cond_wait(&c, &lock);
  }
  numInCriticalSection++;
  pthread_mutex_unlock(&lock);

  // Critical section (up to 3 concurrent threads)
  printf("%c is in the critical section\n", *letter);
  sleep(1);
  printf("%c has left the critical section\n", *letter);

  // Exit protocol: decrement active count and wake one waiting thread
  pthread_mutex_lock(&lock);
  numInCriticalSection--;
  pthread_cond_signal(&c);
  pthread_mutex_unlock(&lock);

  return NULL;
}

int main(int argc, char **argv)
{
  pthread_t threads[8];
  int i;
  char *letters = "abcdefgh";

  for(i = 0; i < 8; ++i)
  {
    pthread_create(&threads[i], NULL, thread, &letters[i]);

    if(i == 4)
      sleep(4);
  }

  for(i = 0; i < 8; i++)
  {
    pthread_join(threads[i], NULL);
  }

  printf("Everything finished...\n");

  return 0;
}