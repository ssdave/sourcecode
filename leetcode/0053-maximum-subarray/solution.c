/* ============================================================================
 * LeetCode #53: Maximum Subarray (Medium)
 * URL:      https://leetcode.com/problems/maximum-subarray/
 * Language: C
 *
 * What is being built / Approach:
 *   Finds the contiguous subarray with the largest sum using Kadane's algorithm
 *   in a single linear pass.
 *   Time Complexity: O(n) | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: nums = [-2, 1, -3, 4, -1, 2, 1, -5, 4] -> 6 ([4, -1, 2, 1])
 *   - Example 2: nums = [1]                             -> 1
 *   - Example 3: nums = [5, 4, -1, 7, 8]                -> 23
 * ============================================================================ */

#include <stdio.h>
#include <assert.h>

#define MAX(a, b) ((a > b) ? a : b)

int maxSubArray(int *nums, int numsSize)
{
    int maxSum = nums[0];
    int curSum = nums[0];

    for(int i = 1; i < numsSize; i++)
    {
        curSum = MAX(curSum + nums[i], nums[i]);
        maxSum = MAX(maxSum, curSum);
    }
    return maxSum;
}

int main(void)
{
    printf("=== LeetCode #53: Maximum Subarray ===\n");

    int nums1[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    int r1 = maxSubArray(nums1, 9);
    printf("Example 1: [-2,1,-3,4,-1,2,1,-5,4] -> %d\n", r1);
    assert(r1 == 6);

    int nums2[] = {1};
    int r2 = maxSubArray(nums2, 1);
    printf("Example 2: [1]                     -> %d\n", r2);
    assert(r2 == 1);

    int nums3[] = {5, 4, -1, 7, 8};
    int r3 = maxSubArray(nums3, 5);
    printf("Example 3: [5,4,-1,7,8]            -> %d\n", r3);
    assert(r3 == 23);

    printf("All tests passed!\n");
    return 0;
}
