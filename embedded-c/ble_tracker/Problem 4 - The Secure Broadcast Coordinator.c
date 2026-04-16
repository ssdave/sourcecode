/* ============================================================================
 * Problem 4: The Secure Broadcast Coordinator
 *
 * The Scenario:
 *   A BLE proximity tracking device has three concurrent actors:
 *     1. The Crypto Engine (`crypto_thread`): Background producer thread that
 *        rotates the 28-byte P-224 Public Key (`current_public_key`) every 5ms.
 *     2. The Motion Sensor (`sensor_thread`): ISR simulator thread that detects
 *        movement every 25ms and signals the BLE radio to broadcast immediately.
 *     3. The BLE Radio (`ble_radio_thread`): Consumer thread that sleeps until
 *        signaled, copies `current_public_key`, and broadcasts it.
 *
 * What is being built:
 *   Thread-safe coordination using `pthread_mutex_t key_mutex` and
 *   `pthread_cond_t broadcast_signal` to prevent torn reads of the 28-byte
 *   public key during key rotation and to wake the radio thread on motion events.
 *
 * What is being tested:
 *   Runs all 3 threads concurrently for 1 second with deliberate 50us per-byte
 *   delays during key writes to maximize torn-read race probability; verifies
 *   `torn_read_count == 0` and `successful_broadcasts > 0`.
 * ============================================================================ */

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// --- Shared Resources ---
uint8_t current_public_key[28];
int broadcast_trigger = 0;

// --- Synchronization Primitives ---
pthread_mutex_t key_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t broadcast_signal = PTHREAD_COND_INITIALIZER;

// --- Test Harness Variables ---
int torn_read_count = 0;
int successful_broadcasts = 0;
int simulation_running = 1;

/* --------------------------------------------------------------------------
 * Thread 1: The Crypto Engine (Producer)
 * -------------------------------------------------------------------------- */
void *crypto_thread(void *arg)
{
  uint8_t mock_key_val = 0x01;

  while(simulation_running)
  {

    // Simulating the key overwrite byte-by-byte to increase the chance
    // of a race condition if unlocked.
    pthread_mutex_lock(&key_mutex);
    for(int i = 0; i < 28; i++)
    {
      current_public_key[i] = mock_key_val;
      usleep(50);
    }
    pthread_mutex_unlock(&key_mutex);

    mock_key_val++;

    usleep(5000); // Wait 5ms before rotating the key again
  }
  return NULL;
}

/* --------------------------------------------------------------------------
 * Thread 2: The Motion Sensor (Signaler)
 * -------------------------------------------------------------------------- */
void *sensor_thread(void *arg)
{
  while(simulation_running)
  {
    usleep(25000); // Simulate movement every 25ms

    // Insert logic to trigger the broadcast and signal the radio
    pthread_mutex_lock(&key_mutex);
    broadcast_trigger = 1;
    pthread_cond_signal(&broadcast_signal);
    pthread_mutex_unlock(&key_mutex);
  }
  return NULL;
}

/* --------------------------------------------------------------------------
 * Thread 3: The BLE Radio Broadcaster (Consumer / Test Harness)
 * -------------------------------------------------------------------------- */
void *ble_radio_thread(void *arg)
{
  uint8_t local_key_buffer[28];

  while(simulation_running)
  {
    pthread_mutex_lock(&key_mutex);

    // Insert logic to wait for the broadcast trigger
    while(broadcast_trigger == 0 && simulation_running)
    {
      pthread_cond_wait(&broadcast_signal, &key_mutex);
    }

    // We are awake and hold the lock! Time to copy the payload safely.
    memcpy(local_key_buffer, current_public_key, 28);

    // Reset the trigger so we don't infinitely broadcast
    broadcast_trigger = 0;

    // Insert logic to release resources
    pthread_mutex_unlock(&key_mutex);

    // --- VERIFICATION ENGINE ---
    int is_corrupted = 0;
    for(int i = 1; i < 28; i++)
    {
      if(local_key_buffer[i] != local_key_buffer[0])
      {
        is_corrupted = 1;
        break;
      }
    }

    if(is_corrupted)
    {
      printf("[FAIL] Torn Read Detected! The array was not protected.\n");
      torn_read_count++;
    }
    else
    {
      successful_broadcasts++;
    }
  }
  return NULL;
}

/* --------------------------------------------------------------------------
 * Main Entry Point
 * -------------------------------------------------------------------------- */
int main()
{
  printf("--- Concurrency Verification Starting ---\n");
  memset(current_public_key, 0x00, 28);

  pthread_t t_crypto, t_sensor, t_ble;

  pthread_create(&t_ble, NULL, ble_radio_thread, NULL);
  pthread_create(&t_crypto, NULL, crypto_thread, NULL);
  pthread_create(&t_sensor, NULL, sensor_thread, NULL);

  // Let the simulation run for 1 full second
  sleep(1);

  // Stop the threads safely
  simulation_running = 0;

  // Wait for threads to finish
  pthread_join(t_crypto, NULL);
  pthread_join(t_sensor, NULL);

  // Wake up the BLE thread one last time so it can exit cleanly.
  pthread_mutex_lock(&key_mutex);
  broadcast_trigger = 1;
  pthread_cond_signal(&broadcast_signal);
  pthread_mutex_unlock(&key_mutex);
  pthread_join(t_ble, NULL);

  // Print Final Results
  printf("\n--- Test Results ---\n");
  printf("Successful Broadcasts: %d\n", successful_broadcasts);
  printf("Torn Reads (Data Corruption): %d\n", torn_read_count);

  if(torn_read_count > 0 || successful_broadcasts == 0)
  {
    printf("\n>>> [RESULT: FAIL] Missing or incorrect synchronization primitives.\n");
  }
  else
  {
    printf("\n>>> [RESULT: PASS] Architecture is completely thread-safe!\n");
  }

  return 0;
}
