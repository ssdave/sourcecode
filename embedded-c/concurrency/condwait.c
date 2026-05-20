/* ============================================================================
 * File: condwait.c
 *
 * Attribution & Copyright:
 *   Originates from Rose-Hulman Institute of Technology — CSSE 332: Operating Systems
 *   Original File: activities/condvar_intro/condwait.c
 *   Course Schedule: https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/
 *   Session Notes:   https://www.rose-hulman.edu/class/csse/csse332/2324b/notes/session19/
 *   Lab Archive:     https://github.com/rhit-csse332/csse332-labs-archive
 *   Authors:         RHIT CSSE 332 Faculty (M. Noureddine, M. Hewner, S. Stamm,
 *                    R. Williamson, S. Mohan)
 *   Completed independently for Operating Systems self-study.
 *
 * What is being built:
 *   Custom `thread_join()` and `thread_exit()` primitives implemented using a
 *   POSIX condition variable (`pthread_cond_t`), mutex (`pthread_mutex_t`), and
 *   an explicit state predicate (`bool child_done`) to avoid lost wakeups.
 *
 * What is being tested:
 *   Parent thread spawns a child worker thread (`sleep(5)`) and blocks in
 *   `thread_join()` on condition variable `c` until the child sets
 *   `child_done = true` and signals `c` inside `thread_exit()`.
 * ============================================================================ */

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * In this exercise, we will try to again simulate pthread_join using condition
 * variables. We will create a thread and then implement the thread_join
 * function below.
 *
 * https://www.rose-hulman.edu/class/csse/csse332/2324b/notes/session19/
 *
 */

pthread_cond_t c = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
bool child_done;

/*
 * exit from a given thread
 */
void thread_exit(void)
{
  // Hold mutex m while updating state predicate child_done so a wakeup is never lost
  pthread_mutex_lock(&m);
  child_done = true;
  pthread_cond_signal(&c);
  pthread_mutex_unlock(&m);
}

void *child(void *arg)
{
  printf("Child\n");
  sleep(5);
  printf("Child completed\n");
  thread_exit();

  return NULL;
}

/**
 * thread_join - Wait for the thread that we create in main by using a condition
 * variable.
 */
void thread_join(void)
{
  // Re-check !child_done in a while loop to guard against spurious wakeups
  pthread_mutex_lock(&m);
  while(!child_done)
  {
    pthread_cond_wait(&c, &m);
  }
  pthread_mutex_unlock(&m);
}

int main(int argc, char **argv)
{
  pthread_t thread;

  pthread_create(&thread, 0, child, NULL);

  printf("Parent waiting for the child to finish\n");
  thread_join();

  /* done */
  exit(0);
}