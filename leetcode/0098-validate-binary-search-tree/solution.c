/* ============================================================================
 * LeetCode #98: Validate Binary Search Tree (Medium)
 * URL:      https://leetcode.com/problems/validate-binary-search-tree/
 * Language: C
 *
 * What is being built / Approach:
 *   Validates whether a binary tree is a valid Binary Search Tree by checking
 *   that the maximum value in the left subtree is strictly less than `root->val`,
 *   the minimum value in the right subtree is strictly greater than `root->val`,
 *   and both subtrees are recursively valid BSTs.
 *   Time Complexity: O(n log n) average | Space Complexity: O(h)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: root = [2, 1, 3]                     -> true
 *   - Example 2: root = [5, 1, 4, null, null, 3, 6]   -> false
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
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

int maxValue(struct TreeNode *root)
{
    if(root == NULL)
    {
        return INT_MIN;
    }
    int max = 0;
    while(root != NULL)
    {
        max = root->val;
        root = root->right;
    }
    return max;
}

int minValue(struct TreeNode *root)
{
    if(root == NULL)
    {
        return INT_MAX;
    }
    int min = 0;
    while(root != NULL)
    {
        min = root->val;
        root = root->left;
    }
    return min;
}

bool isValidBST(struct TreeNode *root)
{
    if(root == NULL)
    {
        return true;
    }
    if((root->left != NULL) && (maxValue(root->left) >= root->val))
    {
        return false;
    }
    if((root->right != NULL) && (minValue(root->right) <= root->val))
    {
        return false;
    }
    if(!(isValidBST(root->left)) || !(isValidBST(root->right)))
    {
        return false;
    }
    return true;
}

int main(void)
{
    printf("=== LeetCode #98: Validate Binary Search Tree ===\n");

    struct TreeNode *t1 = createNode(2);
    t1->left = createNode(1);
    t1->right = createNode(3);
    bool r1 = isValidBST(t1);
    printf("Example 1: root=[2, 1, 3] -> %s\n", r1 ? "true" : "false");
    assert(r1 == true);
    freeTree(t1);

    struct TreeNode *t2 = createNode(5);
    t2->left = createNode(1);
    t2->right = createNode(4);
    t2->right->left = createNode(3);
    t2->right->right = createNode(6);
    bool r2 = isValidBST(t2);
    printf("Example 2: root=[5, 1, 4, null, null, 3, 6] -> %s\n", r2 ? "true" : "false");
    assert(r2 == false);
    freeTree(t2);

    printf("All tests passed!\n");
    return 0;
}
