/* ============================================================================
 * LeetCode #26: Remove Duplicates from Sorted Array (Easy)
 * URL:      https://leetcode.com/problems/remove-duplicates-from-sorted-array/
 * Language: C
 *
 * What is being built / Approach:
 *   Removes duplicates in-place from a sorted array so that each unique element
 *   appears once at the front of `nums`, returning the count of unique elements.
 *   Time Complexity: O(n^2) | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: nums = [1, 1, 2] -> k = 2, nums = [1, 2]
 *   - Example 2: nums = [0, 0, 1, 1, 1, 2, 2, 3, 3, 4] -> k = 5, nums = [0, 1, 2, 3, 4]
 * ============================================================================ */

#include <stdio.h>
#include <limits.h>
#include <assert.h>

int removeDuplicates(int *nums, int numsSize)
{
    if((nums == NULL) || (numsSize == 0))
    {
        return 0;
    }
    if(numsSize == 1)
    {
        return 1;
    }
    if(numsSize == 2)
    {
        if(nums[0] == nums[1])
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }

    int start = 0;
    int unique = 1;
    for(int iter = 1; iter < numsSize; iter++)
    {
        if(nums[start] == nums[iter])
        {
            nums[iter] = INT_MIN;
        }
        else
        {
            start = iter;
            unique++;
        }
    }

    for(int iter = 0; iter < numsSize; iter++)
    {
        if(nums[iter] == INT_MIN)
        {
            for(int j = iter; j < numsSize; j++)
            {
                if(nums[j] != INT_MIN)
                {
                    nums[iter] = nums[j];
                    nums[j] = INT_MIN;
                    break;
                }
            }
        }
    }

    return unique;
}

int main(void)
{
    printf("=== LeetCode #26: Remove Duplicates from Sorted Array ===\n");

    int nums1[] = {1, 1, 2};
    int k1 = removeDuplicates(nums1, 3);
    printf("Example 1: k=%d, nums=[%d, %d]\n", k1, nums1[0], nums1[1]);
    assert(k1 == 2 && nums1[0] == 1 && nums1[1] == 2);

    int nums2[] = {0, 0, 1, 1, 1, 2, 2, 3, 3, 4};
    int exp2[] = {0, 1, 2, 3, 4};
    int k2 = removeDuplicates(nums2, 10);
    printf("Example 2: k=%d, nums=[%d, %d, %d, %d, %d]\n",
           k2, nums2[0], nums2[1], nums2[2], nums2[3], nums2[4]);
    assert(k2 == 5);
    for(int i = 0; i < k2; i++)
    {
        assert(nums2[i] == exp2[i]);
    }

    printf("All tests passed!\n");
    return 0;
}
