/* ============================================================================
 * File: binary_search.c (Practice Harness / Starter Template)
 *
 * Status:
 *   Practice harness kept for timed self-practice: `binary_search` is still a
 *   stub that returns `-1`, so most cases currently print [FAIL]. Because of
 *   that it is built by `make` but is NOT part of `make test`
 *   (run it with `make run FILE=binary_search`).
 *
 * What is being built:
 *   Iterative O(log n) time, O(1) space binary search (`binary_search`) over
 *   an ascending-sorted integer array, using overflow-safe midpoint calculation
 *   (`low + (high - low) / 2`).
 *
 * What is being tested:
 *   Table of boundary cases covering:
 *     - Standard odd/even length arrays (first, middle, last, and missing elements).
 *     - Boundary cases (empty array, NULL pointer, negative size, 1-element array).
 *     - `INT_MIN` / `INT_MAX` extremes, sparse/dense data, and duplicate elements.
 *     - Sub-array pointer offsets (no reliance on hard-coded indices).
 * ============================================================================ */

#include <limits.h>
#include <stdio.h>

// Practice stub: returns the index of `target` in `arr[0..size-1]`, or -1.
int binary_search(const int arr[], int size, int target)
{
  (void)arr;
  (void)size;
  (void)target;
  return -1;
}

// --- Test harness ---

// Standard strict-equality test runner
void run_test(const char *test_name, const int arr[], int size, int target, int expected)
{
  int result = binary_search(arr, size, target);
  if(result == expected)
  {
    printf("[PASS] %s\n", test_name);
  }
  else
  {
    printf("[FAIL] %s - Expected: %d, Got: %d\n", test_name, expected, result);
  }
}

// Special test runner for arrays with duplicates (allows ANY valid matching index)
void run_test_duplicate(const char *test_name, const int arr[], int size, int target)
{
  int result = binary_search(arr, size, target);
  if(result >= 0 && result < size && arr[result] == target)
  {
    printf("[PASS] %s\n", test_name);
  }
  else
  {
    printf("[FAIL] %s - Expected any index containing %d, Got: %d\n", test_name, target, result);
  }
}

int main()
{
  printf("Running Core Test Suite...\n");
  printf("--------------------------\n");

  // 1. Happy Path
  int arr1[] = {1, 2, 3, 4, 5};
  run_test("Target in middle", arr1, 5, 3, 2);
  run_test("Target at start", arr1, 5, 1, 0);
  run_test("Target at end", arr1, 5, 5, 4);

  int arr2[] = {10, 20, 30, 40};
  run_test("Even elements (left mid)", arr2, 4, 20, 1);
  run_test("Even elements (right mid)", arr2, 4, 30, 2);

  // 2. Unhappy Path
  int arr3[] = {10, 20, 30};
  run_test("Target smaller than smallest", arr3, 3, 5, -1);
  run_test("Target larger than largest", arr3, 3, 40, -1);
  run_test("Target between elements", arr3, 3, 15, -1);

  // 3. Basic Edge Cases
  int dummy[] = {1};
  run_test("Empty array (size 0)", dummy, 0, 5, -1);
  run_test("Null pointer", NULL, 10, 5, -1);

  int arr4[] = {7};
  run_test("Single element (match)", arr4, 1, 7, 0);
  run_test("Single element (miss)", arr4, 1, 3, -1);

  // 4. Extreme Limits
  int arr5[] = {INT_MIN, 0, INT_MAX};
  run_test("Find INT_MAX", arr5, 3, INT_MAX, 2);
  run_test("Find INT_MIN", arr5, 3, INT_MIN, 0);

  printf("\nRunning Advanced Logic Breakers...\n");
  printf("----------------------------------\n");

  // 5. Malformed Inputs (Defensive Programming)
  int valid_arr[] = {1, 2, 3};
  run_test("Negative array size", valid_arr, -5, 2, -1);

  // 6. Data Distribution Extremes
  int arr_sparse[] = {-1000000, -5, 0, 99999, 1000000};
  run_test("Sparse array (massive gaps)", arr_sparse, 5, -5, 1);

  int arr_dense[] = {100, 101, 102, 103, 104};
  run_test("Dense array (tightly packed)", arr_dense, 5, 103, 3);

  // 7. Duplicate Handling (Requires special runner)
  int arr_dup[] = {2, 4, 4, 4, 6};
  run_test_duplicate("Array with duplicates (match)", arr_dup, 5, 4);
  run_test("Array with duplicates (miss)", arr_dup, 5, 5, -1);

  int arr_all_same[] = {0, 0, 0, 0, 0};
  run_test_duplicate("Array with identical elements", arr_all_same, 5, 0);

  // 8. Offset/Pointer Arithmetic test (implementation must not rely on hard-coded indices)
  int arr_large[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  run_test("Sub-array pointer offset", arr_large + 5, 5, 8, 2);
  // ^ Passes `arr` starting at value 6. Size is 5. Target 8 should be index 2 relative to the new
  // pointer.

  return 0;
}