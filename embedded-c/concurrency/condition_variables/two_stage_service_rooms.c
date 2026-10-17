/* ============================================================================
 * File: two_stage_service_rooms.c
 *
 * Attribution & Copyright:
 *   Originates from Rose-Hulman Institute of Technology — CSSE 332: Operating Systems
 *   Original File: condvar_basics/rooms.c
 *   Course Schedule: https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/
 *   Lab Spec:        https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/condvar/
 *   Lab Archive:     https://github.com/rhit-csse332/csse332-labs-archive
 *   Authors:         RHIT CSSE 332 Faculty (M. Noureddine, M. Hewner, S. Stamm,
 *                    R. Williamson, S. Mohan)
 *   Completed independently for Operating Systems self-study.
 *
 * What is being built:
 *   Two-stage customer service state machine using two mutexes (`firstlock`,
 *   `secondlock`) and two condition variables (`firstroom`, `secondroom`):
 *     - First Room: capacity 5 (`FIRST_ROOM_CAPACITY`), 1s service time.
 *     - Second Room: capacity 2 (`SECOND_ROOM_CAPACITY`), 2s service time,
 *       with a bounded waiting room of capacity 2 (`WAITING_ROOM_CAPACITY`).
 *     - Customers arriving at the second stage when both the second room and its
 *       waiting room are full immediately leave unhappy (`num_left_customers++`).
 *
 * What is being tested:
 *   Simulates `NUM_PEOPLE_ARRIVING` (10) concurrent customer threads arriving at
 *   randomized intervals; verifies room capacity constraints and tracks unhappy
 *   departures without race conditions or deadlocks.
 * ============================================================================ */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/*
https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/condvar/
*/

// Constants in the code
#define FIRST_ROOM_CAPACITY 5
#define FIRST_ROOM_SERVICE_RATE 1

#define SECOND_ROOM_CAPACITY 2
#define SECOND_ROOM_SERVICE_RATE 2
#define WAITING_ROOM_CAPACITY 2

#define NUM_PEOPLE_ARRIVING 10

// capture the number of customers that leave early
int num_left_customers = 0;
// Occupancy counters for Room 1, Room 2's waiting room, and Room 2
int num_customer_firstroom = 0;
int num_customer_wait_secondroom = 0;
int num_customer_secondroom = 0;
// Separate mutexes and condition variables for each stage to avoid unnecessary contention
pthread_cond_t firstroom = PTHREAD_COND_INITIALIZER;
pthread_cond_t secondroom = PTHREAD_COND_INITIALIZER;
pthread_mutex_t firstlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t secondlock = PTHREAD_MUTEX_INITIALIZER;

void *customer(void *arg)
{
  long int tid = (long int)arg;

  printf("[Customer %ld] Just arrived at first room...\n", tid);

  pthread_mutex_lock(&firstlock);
  while(num_customer_firstroom >= 5)
  {
    pthread_cond_wait(&firstroom, &firstlock);
  }
  num_customer_firstroom++;
  // Enter the first room.
  pthread_mutex_unlock(&firstlock);
  printf("[Customer %ld] Entered first room...\n", tid);
  sleep(FIRST_ROOM_SERVICE_RATE);
  pthread_mutex_lock(&firstlock);
  num_customer_firstroom--;
  pthread_cond_signal(&firstroom);
  pthread_mutex_unlock(&firstlock);
  printf("[Customer %ld] Left first room...\n", tid);

  // Stage 2: Attempt entry into second room or waiting room
  pthread_mutex_lock(&secondlock);
  if((num_customer_wait_secondroom == 2) && (num_customer_secondroom == 2))
  {
    printf("[Customer %ld] Left unhappy because waiting room is full...\n", tid);
    num_left_customers++;
    pthread_mutex_unlock(&secondlock);
    return 0;
  }

  if(num_customer_secondroom == 2)
  {
    num_customer_wait_secondroom++;
    // Note: num_customer_wait_secondroom--; is placed inside the while loop; on
    // spurious wakeup, it decrements the waiting counter multiple times. It should
    // be decremented once after exiting the while loop.
    while(num_customer_secondroom == 2)
    {
      printf("[Customer %ld] Joined the waiting room for second room...\n", tid);
      pthread_cond_wait(&secondroom, &secondlock);
      num_customer_wait_secondroom--;
    }
  }

  // Enter the second room
  num_customer_secondroom++;
  pthread_mutex_unlock(&secondlock);
  printf("[Customer %ld] Entered second room...\n", tid);
  sleep(SECOND_ROOM_SERVICE_RATE);

  pthread_mutex_lock(&secondlock);
  num_customer_secondroom--;
  pthread_cond_signal(&secondroom);
  pthread_mutex_unlock(&secondlock);
  printf("[Customer %ld] Left second room...\n", tid);

  // Done, time to leave...
  return 0;
}

int main(int argc, char **argv)
{
  int i;
  long int tids[NUM_PEOPLE_ARRIVING];
  pthread_t threads[NUM_PEOPLE_ARRIVING];
  srand(time(0));

  for(i = 0; i < NUM_PEOPLE_ARRIVING; i++)
  {
    tids[i] = i + 1;
    pthread_create(&threads[i], 0, customer, (void *)tids[i]);
    if(!(i % 2))
    {
      sleep(rand() % 2);
    }
  }

  for(i = 0; i < NUM_PEOPLE_ARRIVING; i++)
  {
    pthread_join(threads[i], 0);
  }

  printf("Everything finished: %d customers left unhappy...\n", num_left_customers);
  exit(EXIT_SUCCESS);
}