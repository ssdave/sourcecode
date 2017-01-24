/* ============================================================================
 * LeetCode #104: Maximum Depth of Binary Tree (Easy)
 * URL:      https://leetcode.com/problems/maximum-depth-of-binary-tree/
 * Language: C
 *
 * What is being built / Approach:
 *   Computes the maximum depth (height) of a binary tree recursively as
 *   `1 + max(maxDepth(left), maxDepth(right))`.
 *   Time Complexity: O(n) | Space Complexity: O(h)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: root = [3, 9, 20, null, null, 15, 7] -> 3
 *   - Example 2: root = [1, null, 2]                  -> 2
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct TreeNode
{
    int val;
    struct TreeNode *left;
    struct TreeNode *right;
};

static struct TreeNode *createNode(int val)
{
    struct TreeNode *node = (struct TreeNode *)malloc(sizeof(struct TreeNode));
    node->val = val;
    node->left = NULL;
    node->right = NULL;
    return node;
}

static void freeTree(struct TreeNode *root)
{
    if(root == NULL)
    {
        return;
    }
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

#define max(a, b) (a > b ? a : b)

int maxDepth(struct TreeNode *root)
{
    if(!root)
    {
        return 0;
    }
    else
    {
        int ldepth = maxDepth(root->left);
        int rdepth = maxDepth(root->right);

        return (max(ldepth, rdepth) + 1);
    }
}

int main(void)
{
    printf("=== LeetCode #104: Maximum Depth of Binary Tree ===\n");

    struct TreeNode *t1 = createNode(3);
    t1->left = createNode(9);
    t1->right = createNode(20);
    t1->right->left = createNode(15);
    t1->right->right = createNode(7);
    int r1 = maxDepth(t1);
    printf("Example 1: root=[3, 9, 20, null, null, 15, 7] -> %d\n", r1);
    assert(r1 == 3);
    freeTree(t1);

    struct TreeNode *t2 = createNode(1);
    t2->right = createNode(2);
    int r2 = maxDepth(t2);
    printf("Example 2: root=[1, null, 2]                  -> %d\n", r2);
    assert(r2 == 2);
    freeTree(t2);

    printf("All tests passed!\n");
    return 0;
}
