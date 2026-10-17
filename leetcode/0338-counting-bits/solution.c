/* ============================================================================
 * LeetCode #338: Counting Bits (Easy)
 * URL:      https://leetcode.com/problems/counting-bits/
 * Language: C
 *
 * What is being built / Approach:
 *   Returns an array `ans` of length `num + 1` where `ans[i]` is the number of
 *   1 bits in the binary representation of `i`.
 *   Time Complexity: O(n log n) | Space Complexity: O(n)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: n = 2 -> [0, 1, 1]
 *   - Example 2: n = 5 -> [0, 1, 1, 2, 1, 2]
 * ============================================================================ */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int countNumOfBits(int num)
{
  int count = 0;
  while(num != 0)
  {
    num = num & (num - 1);
    count++;
  }
  return count;
}

/**
 * Return an array of size *returnSize.
 * Note: The returned array must be malloced, assume caller calls free().
 */
int *countBits(int num, int *returnSize)
{
  int *ptr = (int *)malloc(sizeof(int) * (num + 1));
  *returnSize = num + 1;

  ptr[0] = 0;
  for(int i = 1; i <= num; i++)
  {
    ptr[i] = countNumOfBits(i);
  }

  return ptr;
}

int main(void)
{
  printf("=== LeetCode #338: Counting Bits ===\n");

  int sz1 = 0;
  int exp1[] = {0, 1, 1};
  int *res1 = countBits(2, &sz1);
  printf("Example 1: n=2 -> [%d, %d, %d]\n", res1[0], res1[1], res1[2]);
  assert(sz1 == 3);
  for(int i = 0; i < sz1; i++)
  {
    assert(res1[i] == exp1[i]);
  }
  free(res1);

  int sz2 = 0;
  int exp2[] = {0, 1, 1, 2, 1, 2};
  int *res2 = countBits(5, &sz2);
  printf("Example 2: n=5 -> [%d, %d, %d, %d, %d, %d]\n", res2[0], res2[1], res2[2], res2[3],
         res2[4], res2[5]);
  assert(sz2 == 6);
  for(int i = 0; i < sz2; i++)
  {
    assert(res2[i] == exp2[i]);
  }
  free(res2);

  printf("All tests passed!\n");
  return 0;
}
