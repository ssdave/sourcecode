/* ============================================================================
 * LeetCode #485: Max Consecutive Ones (Easy)
 * URL:      https://leetcode.com/problems/max-consecutive-ones/
 * Language: C
 *
 * What is being built / Approach:
 *   Finds the maximum number of consecutive 1s in a binary array in a single
 *   pass by incrementing `curr` on 1 and resetting `curr = 0` on 0.
 *   Time Complexity: O(n) | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: nums = [1, 1, 0, 1, 1, 1] -> 3
 *   - Example 2: nums = [1, 0, 1, 1, 0, 1] -> 2
 * ============================================================================ */

#include <stdio.h>
#include <assert.h>

int findMaxConsecutiveOnes(int *nums, int numsSize)
{
    int count = 0;
    int max = 0;
    int curr = 0;
    if(numsSize == 1)
    {
        if(nums[0] == 1)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    while(count < numsSize)
    {
        if(nums[count] == 1)
        {
            curr++;
        }
        else
        {
            curr = 0;
        }
        if(curr > max)
        {
            max = curr;
        }
        count++;
    }
    return max;
}

int main(void)
{
    printf("=== LeetCode #485: Max Consecutive Ones ===\n");

    int nums1[] = {1, 1, 0, 1, 1, 1};
    int r1 = findMaxConsecutiveOnes(nums1, 6);
    printf("Example 1: [1,1,0,1,1,1] -> %d\n", r1);
    assert(r1 == 3);

    int nums2[] = {1, 0, 1, 1, 0, 1};
    int r2 = findMaxConsecutiveOnes(nums2, 6);
    printf("Example 2: [1,0,1,1,0,1] -> %d\n", r2);
    assert(r2 == 2);

    printf("All tests passed!\n");
    return 0;
}
