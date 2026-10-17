/* ============================================================================
 * File: condvar_thread_join.c
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
 *   Parent thread spawns a child worker thread and blocks in `thread_join()`
 *   on condition variable `child_cond` until the child sets `child_done = true`
 *   and signals `child_cond` inside `thread_exit()`.
 * ============================================================================ */

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_cond_t child_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t child_mutex = PTHREAD_MUTEX_INITIALIZER;
bool child_done = false;

/*
 * exit from a given thread
 */
void thread_exit(void)
{
  // Hold child_mutex while updating state predicate child_done so a wakeup is never lost
  pthread_mutex_lock(&child_mutex);
  child_done = true;
  pthread_cond_signal(&child_cond);
  pthread_mutex_unlock(&child_mutex);
}

void *child(void *arg)
{
  printf("Child starting work...\n");
  usleep(100000); // 100ms simulation
  printf("Child completed work\n");
  thread_exit();

  return NULL;
}

/**
 * thread_join - Wait for the child thread by sleeping on condition variable.
 */
void thread_join(void)
{
  // Re-check !child_done in a while loop to guard against spurious wakeups
  pthread_mutex_lock(&child_mutex);
  while(!child_done)
  {
    pthread_cond_wait(&child_cond, &child_mutex);
  }
  pthread_mutex_unlock(&child_mutex);
}

int main(int argc, char **argv)
{
  pthread_t thread;

  pthread_create(&thread, 0, child, NULL);

  printf("Parent waiting for the child to finish\n");
  thread_join();

  printf("[PASS] Parent successfully joined child thread via condition variable\n");
  return 0;
}