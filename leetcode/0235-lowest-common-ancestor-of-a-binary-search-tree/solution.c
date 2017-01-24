/* ============================================================================
 * LeetCode #235: Lowest Common Ancestor of a Binary Search Tree (Medium)
 * URL:      https://leetcode.com/problems/lowest-common-ancestor-of-a-binary-search-tree/
 * Language: C
 *
 * What is being built / Approach:
 *   Finds the lowest common ancestor of nodes `p` and `q` in a BST by walking
 *   left when both values are smaller than `root->val` and right when both are
 *   larger.
 *   Time Complexity: O(h) | Space Complexity: O(h)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: root = [6,2,8,0,4,7,9,null,null,3,5], p = 2, q = 8 -> 6
 *   - Example 2: root = [6,2,8,0,4,7,9,null,null,3,5], p = 2, q = 4 -> 2
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

struct TreeNode *lowestCommonAncestor(struct TreeNode *root, struct TreeNode *p, struct TreeNode *q)
{
    if(!root)
    {
        return NULL;
    }
    if((root == p) || (root == q))
    {
        return root;
    }
    if((root->val > p->val) && (root->val < q->val))
    {
        return root;
    }
    if((root->val > p->val) && (root->val > q->val))
    {
        return lowestCommonAncestor(root->left, p, q);
    }
    if((root->val < p->val) && (root->val < q->val))
    {
        return lowestCommonAncestor(root->right, p, q);
    }
    return root;
}

int main(void)
{
    printf("=== LeetCode #235: Lowest Common Ancestor of a Binary Search Tree ===\n");

    struct TreeNode *root = createNode(6);
    root->left = createNode(2);
    root->right = createNode(8);
    root->left->left = createNode(0);
    root->left->right = createNode(4);
    root->left->right->left = createNode(3);
    root->left->right->right = createNode(5);
    root->right->left = createNode(7);
    root->right->right = createNode(9);

    struct TreeNode *lca1 = lowestCommonAncestor(root, root->left, root->right);
    printf("Example 1: p=2, q=8 -> LCA=%d\n", lca1->val);
    assert(lca1 == root && lca1->val == 6);

    struct TreeNode *lca2 = lowestCommonAncestor(root, root->left, root->left->right);
    printf("Example 2: p=2, q=4 -> LCA=%d\n", lca2->val);
    assert(lca2 == root->left && lca2->val == 2);

    freeTree(root);
    printf("All tests passed!\n");
    return 0;
}
