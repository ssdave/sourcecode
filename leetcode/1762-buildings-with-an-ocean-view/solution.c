/* ============================================================================
 * LeetCode #1762: Buildings With an Ocean View (Medium)
 * URL:      https://leetcode.com/problems/buildings-with-an-ocean-view/
 * Language: C
 *
 * What is being built / Approach:
 *   Scans building heights from right (ocean side) to left while tracking the
 *   running maximum height, recording indices strictly taller than all buildings
 *   to their right, then reverses the resulting index array into ascending order.
 *   Time Complexity: O(n) | Space Complexity: O(n)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: heights = [4, 2, 3, 1] -> [0, 2, 3]
 *   - Example 2: heights = [4, 3, 2, 1] -> [0, 1, 2, 3]
 *   - Example 3: heights = [1, 3, 2, 4] -> [3]
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
int *findBuildings(int *heights, int heightsSize, int *returnSize)
{
    int i = 0;
    int *ret_arr = (int *)malloc(sizeof(int) * (heightsSize));
    int ret_arr_index = 0;

    int max_height = INT_MIN;
    for(i = heightsSize - 1; i >= 0; i--)
    {
        if(heights[i] > max_height)
        {
            max_height = heights[i];
            ret_arr[ret_arr_index++] = i;
        }
    }

    *returnSize = ret_arr_index;
    for(i = 0; i < ret_arr_index / 2; i++)
    {
        int tmp = ret_arr[i];
        ret_arr[i] = ret_arr[ret_arr_index - i - 1];
        ret_arr[ret_arr_index - i - 1] = tmp;
    }

    return ret_arr;
}

int main(void)
{
    printf("=== LeetCode #1762: Buildings With an Ocean View ===\n");

    int h1[] = {4, 2, 3, 1}, exp1[] = {0, 2, 3}, sz1 = 0;
    int *r1 = findBuildings(h1, 4, &sz1);
    printf("Example 1: [4,2,3,1] -> [%d, %d, %d]\n", r1[0], r1[1], r1[2]);
    assert(sz1 == 3);
    for(int i = 0; i < sz1; i++)
    {
        assert(r1[i] == exp1[i]);
    }
    free(r1);

    int h2[] = {4, 3, 2, 1}, exp2[] = {0, 1, 2, 3}, sz2 = 0;
    int *r2 = findBuildings(h2, 4, &sz2);
    printf("Example 2: [4,3,2,1] -> [%d, %d, %d, %d]\n", r2[0], r2[1], r2[2], r2[3]);
    assert(sz2 == 4);
    for(int i = 0; i < sz2; i++)
    {
        assert(r2[i] == exp2[i]);
    }
    free(r2);

    int h3[] = {1, 3, 2, 4}, sz3 = 0;
    int *r3 = findBuildings(h3, 4, &sz3);
    printf("Example 3: [1,3,2,4] -> [%d]\n", r3[0]);
    assert(sz3 == 1 && r3[0] == 3);
    free(r3);

    printf("All tests passed!\n");
    return 0;
}
