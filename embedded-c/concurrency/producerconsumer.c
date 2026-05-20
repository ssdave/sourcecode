/* ============================================================================
 * File: producerconsumer.c
 *
 * Attribution & Copyright:
 *   Originates from Rose-Hulman Institute of Technology — CSSE 332: Operating Systems
 *   Original File: activities/prodcons/03_prodcons.c
 *   Course Schedule: https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/
 *   Session Notes:   https://www.rose-hulman.edu/class/csse/csse332/2324b/notes/session20/
 *   Lab Archive:     https://github.com/rhit-csse332/csse332-labs-archive
 *   Authors:         RHIT CSSE 332 Faculty (M. Noureddine, M. Hewner, S. Stamm,
 *                    R. Williamson, S. Mohan)
 *   Completed independently for Operating Systems self-study.
 *
 * What is being built:
 *   Multi-producer, multi-consumer bounded circular buffer (`MAX = 10`) using
 *   two distinct condition variables (`empty` and `full`), a shared mutex, and
 *   Mesa-safe `while` loops around `pthread_cond_wait`.
 *
 * What is being tested:
 *   5 producer threads and 5 consumer threads each produce/consume 100 items
 *   concurrently (`500` total items across a 10-slot circular queue); verifies
 *   zero deadlock, zero buffer overflow/underflow, and clean termination.
 * ============================================================================ */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

/**
 * The goal of this activity is to implement the correct concurrency management
 * for the producer/consumer problem using condition variables.
 */

const int loops = 100;
#define MAX 10
#define NUM_PRODUCERS 5
#define NUM_CONSUMERS 5

int buffer[MAX];
int fill = 0;
int use = 0;
int count = 0;

pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Put a value into the buffer and increment the fill index.
 */
void put(int value)
{
  buffer[fill] = value;
  fill = (fill + 1) % MAX;
  count += 1;
}

/**
 * Get a value from the buffer and increment the use index.
 */
int get()
{
  int value = buffer[use];
  use = (use + 1) % MAX;
  count -= 1;
  return value;
}

/**
 * The producer can fill item in the buffer when empty
 */
void *producer(void *arg)
{
  int id = *(int *)arg; // Extract the ID passed from main
  int i;
  for(i = 0; i < loops; i++)
  {
    pthread_mutex_lock(&lock);
    // Use while (Mesa semantics) so woken producers re-verify an empty slot exists
    while(count == MAX)
    {
      pthread_cond_wait(&empty, &lock);
    }
    put(i);
    // Signal waiting consumers on the separate 'full' condition variable
    pthread_cond_signal(&full);
    printf("Producer %d produced %d\n", id, i);
    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

/**
 * The consumer can consume items from the buffer when full
 */
void *consumer(void *arg)
{
  int id = *(int *)arg; // Extract the ID passed from main
  int value = 0, i = 0;
  for(; i < loops; i++)
  {
    pthread_mutex_lock(&lock);
    // Wait until at least one item is available in the circular buffer
    while(count == 0)
    {
      pthread_cond_wait(&full, &lock);
    }
    value = get();
    // Signal waiting producers on the 'empty' condition variable that a slot freed up
    pthread_cond_signal(&empty);
    printf("Consumer %d consumed %d\n", id, value);
    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

int main(int argc, char **argv)
{
  /* the threads */
  pthread_t producer_threads[NUM_PRODUCERS];
  pthread_t consumer_threads[NUM_CONSUMERS];

  // Arrays to hold the logical IDs for our threads
  int prod_ids[NUM_PRODUCERS];
  int cons_ids[NUM_CONSUMERS];

  /* create the producer threads */
  for(int i = 0; i < NUM_PRODUCERS; ++i)
  {
    prod_ids[i] = i; // Assign ID 0, 1, 2, 3, 4
    pthread_create(&producer_threads[i], 0, producer, &prod_ids[i]);
  }

  /* create the consumer threads */
  for(int i = 0; i < NUM_CONSUMERS; ++i)
  {
    cons_ids[i] = i; // Assign ID 0, 1, 2, 3, 4
    pthread_create(&consumer_threads[i], 0, consumer, &cons_ids[i]);
  }

  /* wait for producers */
  for(int i = 0; i < NUM_PRODUCERS; ++i)
    pthread_join(producer_threads[i], NULL);

  /* wait for consumers */
  for(int i = 0; i < NUM_CONSUMERS; ++i)
    pthread_join(consumer_threads[i], NULL);
}