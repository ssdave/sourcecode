/* ============================================================================
 * File: mutex_channel_lock_ordering.c
 *
 * Attribution & Copyright:
 *   Originates from Rose-Hulman Institute of Technology — CSSE 332: Operating Systems
 *   Original File: threads_basics/basic_mutex/red_blue_purple.c
 *   Course Schedule: https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/
 *   Lab Spec:        https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/threads/
 *   Lab Archive:     https://github.com/rhit-csse332/csse332-labs-archive
 *   Authors:         RHIT CSSE 332 Faculty (M. Noureddine, M. Hewner, S. Stamm,
 *                    R. Williamson, S. Mohan)
 *   Completed independently for Operating Systems self-study.
 *
 * What is being built:
 *   Fine-grained multi-resource synchronization across two independent output
 *   channels (Red and Blue) using separate mutexes (`redLock`, `blueLock`) and
 *   consistent lock acquisition ordering in `purpleCommand()` to prevent deadlock
 *   while allowing Red-only and Blue-only commands to run in parallel.
 *
 * What is being tested:
 *   Runs 3 concurrent threads executing mixed sequences of White (no lock),
 *   Red (`redLock`), Blue (`blueLock`), and Purple (`blueLock` + `redLock`)
 *   commands; verifies that `Start: <Color>` and `End  : <Color>` pairs never
 *   interleave on the same channel and never deadlock.
 * ============================================================================ */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

/*
 * Multi-resource channel synchronization:
 * - White commands: No channel resource needed.
 * - Red commands: Exclusively acquires redLock.
 * - Blue commands: Exclusively acquires blueLock.
 * - Purple commands: Exclusively acquires both redLock and blueLock in strict
 *   hierarchical order (blueLock -> redLock) to avoid deadlocks.
 */

// Separate mutexes allow Red-only and Blue-only commands to execute concurrently
pthread_mutex_t redLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t blueLock = PTHREAD_MUTEX_INITIALIZER;

void redCommand()
{
  // Acquire only redLock so Blue commands are not blocked
  pthread_mutex_lock(&redLock);
  printf("Start: Red\n");
  usleep(100);
  printf("End  : Red\n");
  pthread_mutex_unlock(&redLock);
}

void blueCommand()
{
  // Acquire only blueLock so Red commands are not blocked
  pthread_mutex_lock(&blueLock);
  printf("Start: Blue\n");
  usleep(100);
  printf("End  : Blue\n");
  pthread_mutex_unlock(&blueLock);
}

void purpleCommand()
{
  // Acquire both channel locks in a fixed global order (blueLock -> redLock)
  // and release in reverse order to prevent circular-wait deadlocks
  pthread_mutex_lock(&blueLock);
  pthread_mutex_lock(&redLock);
  printf("Start: Blue\n");
  printf("Start: Red\n");
  usleep(100);
  printf("End  : Red\n");
  printf("End  : Blue\n");
  pthread_mutex_unlock(&redLock);
  pthread_mutex_unlock(&blueLock);
}

void whiteCommand()
{
  usleep(100);
}

void *thread1(void *p)
{
  redCommand();
  redCommand();
  whiteCommand();
  redCommand();
  purpleCommand();
  redCommand();
  printf("thread 1 done\n");
  pthread_exit(NULL);
}

void *thread2(void *p)
{
  redCommand();
  blueCommand();
  blueCommand();
  whiteCommand();
  purpleCommand();
  redCommand();
  blueCommand();
  printf("thread 2 done\n");
  pthread_exit(NULL);
}

void *thread3(void *p)
{
  redCommand();
  purpleCommand();
  purpleCommand();
  purpleCommand();
  printf("thread 3 done\n");
  pthread_exit(NULL);
}

int main(int argc, char **argv)
{
  pthread_t threads[3];

  // to fully test your code, feel free to try other configurations
  // (e.g. a couple thread1s running at the same time, etc.)
  pthread_create(&threads[0], NULL, thread1, NULL);
  pthread_create(&threads[1], NULL, thread2, NULL);
  pthread_create(&threads[2], NULL, thread3, NULL);

  int i;
  for(i = 0; i < 3; i++)
  {
    pthread_join(threads[i], NULL);
  }

  printf("Everything finished\n");

  return 0;
}