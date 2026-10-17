/* ============================================================================
 * LeetCode #283: Move Zeroes (Easy)
 * URL:      https://leetcode.com/problems/move-zeroes/
 * Language: C
 *
 * What is being built / Approach:
 *   Moves all 0s in `nums` to the end in-place while preserving the relative
 *   order of non-zero elements using two pointers `ptr1` and `ptr2`.
 *   Time Complexity: O(n^2) | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: nums = [0, 1, 0, 3, 12] -> [1, 3, 12, 0, 0]
 *   - Example 2: nums = [0]              -> [0]
 * ============================================================================ */

#include <assert.h>
#include <stdio.h>

void moveZeroes(int *nums, int numsSize)
{
  if((nums == NULL) || (numsSize == 0) || (numsSize == 1))
  {
    return;
  }

  for(int i = 0; i < numsSize - 1; i++)
  {
    int *ptr1 = &nums[i];
    int *ptr2 = &nums[i + 1];
    if(*ptr1 == 0)
    {
      while(*ptr2 == 0)
      {
        if(ptr2 == &nums[numsSize - 1])
        {
          break;
        }
        ptr2++;
      }
      if(*ptr2 == 0)
      {
        break;
      }
      else
      {
        int tmp = *ptr1;
        *ptr1 = *ptr2;
        *ptr2 = tmp;
      }
    }
  }
}

int main(void)
{
  printf("=== LeetCode #283: Move Zeroes ===\n");

  int nums1[] = {0, 1, 0, 3, 12};
  int exp1[] = {1, 3, 12, 0, 0};
  moveZeroes(nums1, 5);
  printf("Example 1: [0,1,0,3,12] -> [%d, %d, %d, %d, %d]\n", nums1[0], nums1[1], nums1[2],
         nums1[3], nums1[4]);
  for(int i = 0; i < 5; i++)
  {
    assert(nums1[i] == exp1[i]);
  }

  int nums2[] = {0};
  moveZeroes(nums2, 1);
  printf("Example 2: [0]          -> [%d]\n", nums2[0]);
  assert(nums2[0] == 0);

  printf("All tests passed!\n");
  return 0;
}
