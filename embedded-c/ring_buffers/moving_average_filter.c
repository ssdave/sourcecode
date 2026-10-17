/* ============================================================================
 * File: moving_average_filter.c
 *
 * What is being built:
 *   O(1) sliding-window moving average filter (`WINDOW_SIZE = 4`) for sensor
 *   data streams. Maintains a circular sample buffer, running `sum`, `head`
 *   index, and sample `count` so each update subtracts the oldest sample and
 *   adds the newest sample without re-summing the window.
 *
 * What is being tested:
 *   Table-driven floating-point test suite verifying:
 *     1. Partial-window ramp-up (first 1..3 samples divide by `count`).
 *     2. Full-window steady state (`count == WINDOW_SIZE`).
 *     3. Circular wrap-around overwriting the oldest sample at `head`.
 *     4. Negative values, zero inputs, and step-response convergence.
 * ============================================================================ */

#include <stdio.h>

#define WINDOW_SIZE 4

typedef struct
{
  float input;
  float expected_avg;
  const char *description;
} TestCase;

typedef struct
{
  float buffer[WINDOW_SIZE];
  int head;  // Points to the oldest element to be overwritten
  float sum; // Tracks the running sum of the current window
  int count; // Tracks how many elements have been added so far
} MovingAverage;

void print_debug(MovingAverage *ma)
{
  printf("🔍 [DEBUG] Buffer: [");
  for(int i = 0; i < WINDOW_SIZE; i++)
  {
    // Print an asterisk next to the element currently at the 'head' index
    if(i == ma->head)
    {
      printf("*%.1f*", ma->buffer[i]);
    }
    else
    {
      printf("%.1f", ma->buffer[i]);
    }
    if(i < WINDOW_SIZE - 1)
    {
      printf(", ");
    }
  }
  printf("] | Head: %d | Sum: %.1f | Count: %d\n", ma->head, ma->sum, ma->count);
}

// Initialize the structure
void init_moving_average(MovingAverage *ma)
{
  for(int i = 0; i < WINDOW_SIZE; i++)
  {
    ma->buffer[i] = 0.0f;
  }
  ma->head = 0;
  ma->sum = 0.0f;
  ma->count = 0;
}

// Add a new value and return the updated moving average
float update_moving_average(MovingAverage *ma, float new_value)
{
  // Write newest sample into the circular slot at head and add to running sum
  ma->buffer[ma->head++] = new_value;
  ma->sum += new_value;
  ma->count++;
  float average = ma->sum / ma->count;

  // Once the window reaches WINDOW_SIZE, wrap head and subtract the oldest sample
  // so the next call can immediately add the incoming sample in O(1) time
  if(ma->count == WINDOW_SIZE)
  {
    ma->count--;
    ma->head = ma->head % WINDOW_SIZE;
    ma->sum -= ma->buffer[ma->head];
  }

  return average;
}

int main()
{
  MovingAverage ma;
  init_moving_average(&ma);

  TestCase tests[] = {{10.0f, 10.0f, "Edge: Initial fill (count 1)"},
                      {20.0f, 15.0f, "Edge: Partial fill (count 2)"},
                      {30.0f, 20.0f, "Edge: Partial fill (count 3)"},
                      {40.0f, 25.0f, "Edge: Exactly full window (count 4)"},
                      {50.0f, 35.0f, "Edge: Sliding window (overwrites oldest 10.0)"},
                      {-10.0f, 27.5f, "Edge: Negative number injection"},
                      {0.0f, 20.0f, "Edge: Zero injection"},
                      {0.0f, 10.0f, "Edge: Wrap-around completion (head resets to 0)"}};

  int num_tests = sizeof(tests) / sizeof(tests[0]);

  printf("--- Testing Moving Average (Window Size: %d) ---\n", WINDOW_SIZE);
  for(int i = 0; i < num_tests; i++)
  {
    float avg = update_moving_average(&ma, tests[i].input);

    // Simple verification check with a small tolerance for float comparison
    if(avg >= tests[i].expected_avg - 0.001f && avg <= tests[i].expected_avg + 0.001f)
    {
      printf("[PASS] ✅ ");
    }
    else
    {
      printf("[FAIL] ❌ ");
    }

    printf("In: %6.2f | Expected: %6.2f | Got: %6.2f | %s\n", tests[i].input, tests[i].expected_avg,
           avg, tests[i].description);

    // print_debug(&ma);
  }

  return 0;
}