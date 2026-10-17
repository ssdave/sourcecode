/* ============================================================================
 * File: ordered_thread_entry.c (CSSE 332 Lab Starter Template)
 *
 * Status:
 *   Unimplemented CSSE 332 lab starter template preserved for practice.
 *   (See `bounded_critical_section.c` and `two_stage_service_rooms.c` in this directory for
 * completed solutions.)
 *
 * Attribution & Copyright:
 *   Originates from Rose-Hulman Institute of Technology — CSSE 332: Operating Systems
 *   Original File: condvar_basics/inorder.c
 *   Course Schedule: https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/
 *   Lab Spec:        https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/condvar/
 *   Lab Archive:     https://github.com/rhit-csse332/csse332-labs-archive
 *   Authors:         RHIT CSSE 332 Faculty (M. Noureddine, M. Hewner, S. Stamm,
 *                    R. Williamson, S. Mohan)
 *
 * What is being built:
 *   Condition-variable thread ordering coordinator that forces threads numbered
 *   1..4 to enter their critical section strictly in ascending numerical order
 *   (1, then 2, then 3, then 4) regardless of thread creation or arrival order.
 *
 * What is being tested:
 *   Spawns 4 threads with out-of-order IDs `{2, 1, 4, 3}` and a 3-second delay
 *   after creating thread 4 (`i == 2`) to test strict sequential execution.
 * ============================================================================ */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

/*
https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/condvar/
*/

void *thread(void *arg)
{
  int *num = (int *)arg;
  printf("%d wants to enter the critical section\n", *num);

  printf("%d is finished with the critical section\n", *num);

  return NULL;
}

int main(int argc, char **argv)
{
  pthread_t threads[4];
  int i;
  int nums[] = {2, 1, 4, 3};

  for(i = 0; i < 4; ++i)
  {
    pthread_create(&threads[i], NULL, thread, &nums[i]);

    if(i == 2)
    {
      sleep(3);
    }
  }

  for(i = 0; i < 4; ++i)
  {
    pthread_join(threads[i], NULL);
  }

  printf("Everything finished\n");

  return 0;
}