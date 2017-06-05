/* ============================================================================
 * LeetCode #226: Invert Binary Tree (Easy)
 * URL:      https://leetcode.com/problems/invert-binary-tree/
 * Language: C
 *
 * What is being built / Approach:
 *   Recursively inverts (mirrors) a binary tree by swapping the left and right
 *   child pointers at every node.
 *   Time Complexity: O(n) | Space Complexity: O(h)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: root = [4, 2, 7, 1, 3, 6, 9] -> [4, 7, 2, 9, 6, 3, 1]
 *   - Example 2: root = [2, 1, 3]             -> [2, 3, 1]
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

struct TreeNode *invertTree(struct TreeNode *root)
{
    if(root == NULL)
    {
        return NULL;
    }
    else
    {
        if(root->left)
        {
            root->left = invertTree(root->left);
        }
        if(root->right)
        {
            root->right = invertTree(root->right);
        }
        if((root->left == NULL) && (root->right == NULL))
        {
            return root;
        }
        struct TreeNode *temp;
        temp = root->left;
        root->left = root->right;
        root->right = temp;
        return root;
    }
}

int main(void)
{
    printf("=== LeetCode #226: Invert Binary Tree ===\n");

    struct TreeNode *t1 = createNode(4);
    t1->left = createNode(2);
    t1->right = createNode(7);
    t1->left->left = createNode(1);
    t1->left->right = createNode(3);
    t1->right->left = createNode(6);
    t1->right->right = createNode(9);
    t1 = invertTree(t1);
    printf("Example 1: root=[4,2,7,1,3,6,9] -> [%d,%d,%d,%d,%d,%d,%d]\n",
           t1->val, t1->left->val, t1->right->val,
           t1->left->left->val, t1->left->right->val,
           t1->right->left->val, t1->right->right->val);
    assert(t1->val == 4 && t1->left->val == 7 && t1->right->val == 2);
    assert(t1->left->left->val == 9 && t1->left->right->val == 6);
    assert(t1->right->left->val == 3 && t1->right->right->val == 1);
    freeTree(t1);

    struct TreeNode *t2 = createNode(2);
    t2->left = createNode(1);
    t2->right = createNode(3);
    t2 = invertTree(t2);
    printf("Example 2: root=[2,1,3]         -> [%d,%d,%d]\n",
           t2->val, t2->left->val, t2->right->val);
    assert(t2->val == 2 && t2->left->val == 3 && t2->right->val == 1);
    freeTree(t2);

    printf("All tests passed!\n");
    return 0;
}
