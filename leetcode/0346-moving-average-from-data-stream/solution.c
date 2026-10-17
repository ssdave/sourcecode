/* ============================================================================
 * LeetCode #346: Moving Average from Data Stream (Easy)
 * URL:      https://leetcode.com/problems/moving-average-from-data-stream/
 * Language: C
 *
 * What is being built / Approach:
 *   Computes the sliding-window moving average of a stream of integers using
 *   a circular buffer array of fixed capacity `size` and a running `sum`.
 *   Time Complexity: O(1) per `next()` | Space Complexity: O(size)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: MovingAverage(3), next(1)->1.0, next(10)->5.5,
 *                next(3)->4.66667, next(5)->6.0
 * ============================================================================ */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
  int size;
  int *arr;
  int sum;
  int count;
  int reset;
} MovingAverage;

MovingAverage *movingAverageCreate(int size)
{
  MovingAverage *obj = malloc(sizeof(MovingAverage));
  obj->size = size;
  obj->arr = malloc(sizeof(int) * size);
  obj->sum = 0;
  obj->count = 0;
  obj->reset = 0;
  return obj;
}

double movingAverageNext(MovingAverage *obj, int val)
{
  if(obj == NULL)
  {
    return 0.0;
  }
  double average = 0.0;
  if(obj->count < obj->size)
  {
    if(obj->reset)
    {
      obj->sum = obj->sum - obj->arr[obj->count];
    }
    obj->arr[obj->count++] = val;
    obj->sum += val;
    average = (double)obj->sum / (obj->reset ? obj->size : obj->count);
  }
  else if(obj->count == obj->size)
  {
    obj->reset = 1;
    obj->count = 0;
    obj->sum = obj->sum - obj->arr[obj->count];
    obj->arr[obj->count++] = val;
    obj->sum += val;
    average = (double)obj->sum / obj->size;
  }
  return (double)average;
}

void movingAverageFree(MovingAverage *obj)
{
  free(obj->arr);
  free(obj);
  obj = NULL;
}

int main(void)
{
  printf("=== LeetCode #346: Moving Average from Data Stream ===\n");

  MovingAverage *obj = movingAverageCreate(3);
  double v1 = movingAverageNext(obj, 1);
  double v2 = movingAverageNext(obj, 10);
  double v3 = movingAverageNext(obj, 3);
  double v4 = movingAverageNext(obj, 5);

  printf("Example 1: next(1)=%.5f, next(10)=%.5f, next(3)=%.5f, next(5)=%.5f\n", v1, v2, v3, v4);
  assert(fabs(v1 - 1.0) < 1e-5);
  assert(fabs(v2 - 5.5) < 1e-5);
  assert(fabs(v3 - (14.0 / 3.0)) < 1e-5);
  assert(fabs(v4 - 6.0) < 1e-5);

  movingAverageFree(obj);
  printf("All tests passed!\n");
  return 0;
}
