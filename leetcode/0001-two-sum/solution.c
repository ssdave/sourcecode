/* ============================================================================
 * LeetCode #1: Two Sum (Easy)
 * URL:      https://leetcode.com/problems/two-sum/
 * Language: C
 *
 * What is being built / Approach:
 *   Finds two indices in an integer array whose values sum to `target` using
 *   a nested scan and returns a dynamically allocated 2-element index array.
 *   Time Complexity: O(n^2) | Space Complexity: O(1) auxiliary
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: nums = [2, 7, 11, 15], target = 9 -> [0, 1]
 *   - Example 2: nums = [3, 2, 4], target = 6      -> [1, 2]
 *   - Example 3: nums = [3, 3], target = 6         -> [0, 1]
 * ============================================================================ */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
int *twoSum(int *nums, int numsSize, int target, int *returnSize)
{
  if(nums == NULL || numsSize == 0)
  {
    return NULL;
  }

  int *ret = (int *)malloc(sizeof(int) * 2);

  for(int index = 0; index < numsSize; index++)
  {
    int sum = nums[index];

    for(int iter = index + 1; iter < numsSize; iter++)
    {
      if((target - sum) == nums[iter])
      {
        ret[0] = index;
        ret[1] = iter;
        sum += nums[iter];
        *returnSize = 2;
        break;
      }
    }
    if(sum == target)
    {
      break;
    }
  }

  return ret;
}

int main(void)
{
  printf("=== LeetCode #1: Two Sum ===\n");

  int nums1[] = {2, 7, 11, 15};
  int retSize1 = 0;
  int *res1 = twoSum(nums1, 4, 9, &retSize1);
  printf("Example 1: nums=[2,7,11,15], target=9 -> [%d, %d]\n", res1[0], res1[1]);
  assert(retSize1 == 2 && res1[0] == 0 && res1[1] == 1);
  free(res1);

  int nums2[] = {3, 2, 4};
  int retSize2 = 0;
  int *res2 = twoSum(nums2, 3, 6, &retSize2);
  printf("Example 2: nums=[3,2,4], target=6     -> [%d, %d]\n", res2[0], res2[1]);
  assert(retSize2 == 2 && res2[0] == 1 && res2[1] == 2);
  free(res2);

  int nums3[] = {3, 3};
  int retSize3 = 0;
  int *res3 = twoSum(nums3, 2, 6, &retSize3);
  printf("Example 3: nums=[3,3], target=6       -> [%d, %d]\n", res3[0], res3[1]);
  assert(retSize3 == 2 && res3[0] == 0 && res3[1] == 1);
  free(res3);

  printf("All tests passed!\n");
  return 0;
}
