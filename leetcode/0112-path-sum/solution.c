/* ============================================================================
 * LeetCode #112: Path Sum (Easy)
 * URL:      https://leetcode.com/problems/path-sum/
 * Language: C
 *
 * What is being built / Approach:
 *   Determines whether a binary tree has a root-to-leaf path whose node values
 *   sum to `sum` by subtracting `root->val` at each level until reaching a leaf.
 *   Time Complexity: O(n) | Space Complexity: O(h)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: root = [5,4,8,11,null,13,4,7,2,null,null,null,1], targetSum = 22 -> true
 *   - Example 2: root = [1, 2, 3], targetSum = 5                                  -> false
 *   - Example 3: root = [],        targetSum = 0                                  -> false
 * ============================================================================ */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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

bool hasPathSum(struct TreeNode *root, int sum)
{
  if(root == NULL)
  {
    return false;
  }
  if((root->left == NULL) && (root->right == NULL))
  {
    if(root->val == sum)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return (hasPathSum(root->left, sum - root->val) || hasPathSum(root->right, sum - root->val));
  }
}

int main(void)
{
  printf("=== LeetCode #112: Path Sum ===\n");

  struct TreeNode *t1 = createNode(5);
  t1->left = createNode(4);
  t1->right = createNode(8);
  t1->left->left = createNode(11);
  t1->left->left->left = createNode(7);
  t1->left->left->right = createNode(2);
  t1->right->left = createNode(13);
  t1->right->right = createNode(4);
  t1->right->right->right = createNode(1);
  bool r1 = hasPathSum(t1, 22);
  printf("Example 1: targetSum=22 -> %s\n", r1 ? "true" : "false");
  assert(r1 == true);
  freeTree(t1);

  struct TreeNode *t2 = createNode(1);
  t2->left = createNode(2);
  t2->right = createNode(3);
  bool r2 = hasPathSum(t2, 5);
  printf("Example 2: targetSum=5  -> %s\n", r2 ? "true" : "false");
  assert(r2 == false);
  freeTree(t2);

  bool r3 = hasPathSum(NULL, 0);
  printf("Example 3: root=[], targetSum=0 -> %s\n", r3 ? "true" : "false");
  assert(r3 == false);

  printf("All tests passed!\n");
  return 0;
}
