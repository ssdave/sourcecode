/* ============================================================================
 * File: prodcons_condvar.c (CSSE 332 Lab Starter Template)
 *
 * Status:
 *   Unimplemented CSSE 332 lab starter template preserved for practice.
 *   (See `../producerconsumer.c`, `max.c`, and `rooms.c` for completed solutions.)
 *
 * Attribution & Copyright:
 *   Originates from Rose-Hulman Institute of Technology — CSSE 332: Operating Systems
 *   Course Schedule: https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/
 *   Lab Spec:        https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/condvar/
 *   Lab Archive:     https://github.com/rhit-csse332/csse332-labs-archive
 *   Authors:         RHIT CSSE 332 Faculty (M. Noureddine, M. Hewner, S. Stamm,
 *                    R. Williamson, S. Mohan)
 *
 * What is being built:
 *   Bounded 5-element stack buffer (`buffer[BUFFERSIZE]`, `last_valid_index`)
 *   shared between 2 fast producer threads and 2 slow consumer threads.
 *
 * What is being tested:
 *   Producers generate values `100..109` and `200..209` while consumers sleep
 *   1 second between reads, exercising bounded-buffer overflow and underflow
 *   synchronization.
 * ============================================================================ */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define BUFFERSIZE 5
int buffer[BUFFERSIZE];
int last_valid_index;

void *producer(void *arg)
{
  int i;
  int value = *((int *)arg);

  for(i = 0; i < 10; ++i)
  {
    buffer[last_valid_index + 1] = value;
    last_valid_index++;

    printf("Produced value %d, stored at %d\n", value, last_valid_index);

    value += 1;
  }

  return NULL;
}

void *consumer(void *arg)
{
  int value, i;

  for(i = 0; i < 10; ++i)
  {
    sleep(1);

    value = buffer[last_valid_index];
    last_valid_index--;
    printf("Consumed value %d, stored at %d\n", value, last_valid_index + 1);
  }

  return NULL;
}

int main(int argc, char **argv)
{
  pthread_t p1, p2, c1, c2;
  int p1start = 100;
  int p2start = 200;

  last_valid_index = -1; // initially, there is no valid data
  pthread_create(&p1, NULL, producer, &p1start);
  pthread_create(&p2, NULL, producer, &p2start);
  pthread_create(&c1, NULL, consumer, NULL);
  pthread_create(&c2, NULL, consumer, NULL);

  pthread_join(p1, NULL);
  pthread_join(p2, NULL);
  pthread_join(c1, NULL);
  pthread_join(c2, NULL);

  printf("Everything finished...\n");
}