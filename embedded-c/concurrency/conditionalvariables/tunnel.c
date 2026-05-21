/* ============================================================================
 * File: tunnel.c (CSSE 332 Lab Starter Template)
 *
 * Status:
 *   Unimplemented CSSE 332 lab starter template preserved for practice.
 *   (See `max.c` and `rooms.c` in this directory for completed solutions.)
 *
 * Attribution & Copyright:
 *   Originates from Rose-Hulman Institute of Technology — CSSE 332: Operating Systems
 *   Original File: condvar_basics/tunnel.c
 *   Course Schedule: https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/
 *   Lab Spec:        https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/condvar/
 *   Lab Archive:     https://github.com/rhit-csse332/csse332-labs-archive
 *   Authors:         RHIT CSSE 332 Faculty (M. Noureddine, M. Hewner, S. Stamm,
 *                    R. Williamson, S. Mohan)
 *
 * What is being built:
 *   Bidirectional traffic tunnel synchronization problem with East-West (`ewcar`),
 *   West-East (`wecar`), and priority emergency `ambulance` threads.
 *
 * What is being tested:
 *   Spawns 15 EW cars (3s transit), 15 WE cars (2s transit), and 7 ambulances
 *   (1s transit, arriving at random intervals) to test directional exclusion,
 *   capacity limits, and ambulance priority entry.
 * ============================================================================ */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// constants to control number of cars and threads
#define NUM_EW_CARS 15
#define NUM_WE_CARS 15
#define NUM_AMBULANCE 7
#define TOTAL_THREADS NUM_EW_CARS + NUM_WE_CARS + NUM_AMBULANCE

// constants to define ambulance direction
#define EW_DIRECTION 1
#define WE_DIRECTION 2

// car speeds
#define EW_SPEED 3
#define WE_SPEED 2

// state variables

// concurrency means

void *ewcar(void *arg)
{
  int tid = *(int *)arg;

  printf("Car (%d) entered tunnel in EW direction...\n", tid);
  sleep(EW_SPEED);
  printf("Car (%d) exited tunnel in EW direction...\n", tid);

  return 0;
}

void *wecar(void *arg)
{
  int tid = *(int *)arg;

  printf("Car (%d) entered tunnel in WE direction...\n", tid);
  sleep(WE_SPEED);
  printf("Car (%d) exited tunnel in WE direction...\n", tid);

  return 0;
}

void *ambulance(void *arg)
{
  int tid = *(int *)arg;
  int direction = EW_DIRECTION;

  if(direction == EW_DIRECTION)
  {
    printf("Ambulance %d entered the tunnel in EW direction\n", tid);
  }
  else if(direction == WE_DIRECTION)
  {
    printf("Ambulance %d entered the tunnel in WE direction\n", tid);
  }

  sleep(1); // ambulance is the fastest

  if(direction == EW_DIRECTION)
  {
    printf("Ambulance %d exited the tunnel in EW direction\n", tid);
  }
  else if(direction == WE_DIRECTION)
  {
    printf("Ambulance %d exited the tunnel in WE direction\n", tid);
  }

  return 0;
}

int main(int argc, char **argv)
{
  pthread_t threads[NUM_EW_CARS + NUM_WE_CARS + NUM_AMBULANCE];
  int tids[NUM_EW_CARS + NUM_WE_CARS + NUM_AMBULANCE];
  int i;
  srand(time(0));

  for(i = 0; i < NUM_EW_CARS; i++)
  {
    tids[i] = i + 1;
    pthread_create(&threads[i], 0, ewcar, &tids[i]);
  }

  for(i = NUM_EW_CARS; i < NUM_WE_CARS + NUM_EW_CARS; i++)
  {
    tids[i] = i + 1;
    pthread_create(&threads[i], 0, wecar, &tids[i]);
  }

  for(i = NUM_EW_CARS + NUM_WE_CARS; i < TOTAL_THREADS; i++)
  {
    tids[i] = i + 1;
    pthread_create(&threads[i], 0, ambulance, &tids[i]);
    // make the ambulances arrive at random times
    sleep(rand() % 10);
  }

  for(i = 0; i < TOTAL_THREADS; i++)
  {
    pthread_join(threads[i], 0);
  }

  printf("Everyting finished...\n");
  exit(EXIT_SUCCESS);
}