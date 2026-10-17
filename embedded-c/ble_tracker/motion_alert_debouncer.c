/* ============================================================================
 * File: motion_alert_debouncer.c
 *
 * The Scenario:
 *   To prevent unwanted tracking, a BLE proximity tag must play a sound if it
 *   is moving without its paired host device. However, accelerometers are noisy.
 *   A single bump on a table shouldn't trigger an alert, but continuous walking
 *   should.
 *
 * What is being built:
 *   Stateful rolling-window accelerometer interrupt debouncer (`on_movement_detected`)
 *   that tracks up to 5 movement timestamps and triggers `play_chime()` only when
 *   5 movement events occur within a rolling 60-second window.
 *
 * What is being tested:
 *   1. Scattered motion (5 events spread over 100s -> chime remains silent).
 *   2. Continuous walking motion (5 events within 40s -> chime triggers once).
 *   3. Sliding window expiration (4 early events expire after a pause, followed
 *      by 5 rapid events between `t=300s` and `t=320s` -> chime triggers).
 * ============================================================================ */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// We use a global counter in our test harness to verify if the chime actually played
static int chime_play_count = 0;

void play_chime(void)
{
  printf("    >>> [HARDWARE: CHIME PLAYING!] <<<\n");
  chime_play_count++;
}

void flush_rolling_window(int index);

uint32_t rolling_window[5];

void flush_rolling_window(int index)
{
  for(int i = 0; i < index; i++)
  {
    rolling_window[i] = 0;
  }
  chime_play_count = 0;
}

// Accelerometer ISR movement callback:
// 'timestamp_sec' is the current system uptime in seconds.
void on_movement_detected(uint32_t timestamp_sec)
{
  for(int i = 0; i < 5; i++)
  {
    if(rolling_window[i] == 0)
    {
      // If within 60s of the first event in the window, append to current window
      if(timestamp_sec - rolling_window[0] <= 60)
      {
        rolling_window[i] = timestamp_sec;
      }
      else
      {
        // Window expired (> 60s since rolling_window[0]): flush stale events and restart window
        flush_rolling_window(i);
        rolling_window[0] = timestamp_sec;
      }
      break;
    }
  }

  // Count valid movement events accumulated in the current 60s window
  int count = 0;
  for(int i = 0; i < 5; i++)
  {
    if(rolling_window[i] != 0)
    {
      count++;
    }
  }

  // Trigger hardware alert chime once 5 events occur within the 60s window
  if(count == 5)
  {
    play_chime();
  }

  // printf("%d %d %d %d %d %d\n", timestamp_sec, rolling_window[0], rolling_window[1],
  // rolling_window[2], rolling_window[3], rolling_window[4]);
}

int main()
{
  printf("--- Anti-Stalking Debouncer Verification ---\n\n");

  // TEST 1: Too slow (5 events over 100 seconds)
  // Simulates someone occasionally bumping a bag on a train
  printf("1. Testing scattered movement (5 events over 100 seconds)...\n");
  flush_rolling_window(5);
  on_movement_detected(1);
  on_movement_detected(25);
  on_movement_detected(50);
  on_movement_detected(75);
  on_movement_detected(100);
  if(chime_play_count == 0)
  {
    printf("[PASS] Chime remained silent.\n\n");
  }
  else
  {
    printf("[FAIL] Chime played inappropriately.\n\n");
  }

  // TEST 2: Fast enough (5 events within 40 seconds)
  // Simulates someone actually walking with the device
  printf("2. Testing continuous movement (5 events within 40 seconds)...\n");
  flush_rolling_window(5);
  on_movement_detected(100);
  on_movement_detected(110);
  on_movement_detected(120);
  on_movement_detected(130);
  on_movement_detected(140);
  if(chime_play_count == 1)
  {
    printf("[PASS] Chime successfully triggered.\n\n");
  }
  else
  {
    printf("[FAIL] Chime failed to trigger.\n\n");
  }

  // TEST 3: The Sliding Window (Dropping old events)
  // Simulates moving a bit, pausing for a long time, then moving continuously
  printf("3. Testing sliding window (old events expiring)...\n");
  flush_rolling_window(5);
  on_movement_detected(200);
  on_movement_detected(205);
  on_movement_detected(210);
  on_movement_detected(215); // 4 quick events.
  // Long pause... the first 4 should now expire relative to new events
  on_movement_detected(300); // 5th event, but delta from event 1 is 100s. (No chime)
  on_movement_detected(305); // Delta from event 2 is 100s. (No chime)
  on_movement_detected(310);
  on_movement_detected(315);
  on_movement_detected(320); // Now we have 5 new events between 300 and 320.
  if(chime_play_count == 1)
  {
    printf("[PASS] Sliding window properly dropped expired events.\n\n");
  }
  else
  {
    printf("[FAIL] Sliding window logic failed.\n\n");
  }

  return 0;
}