# LeetCode Solutions in C (`leetcode`)

26 accepted LeetCode solutions in C, synced with their original submission timestamps (`GIT_AUTHOR_DATE` / `GIT_COMMITTER_DATE`). Every [`solution.c`](./0001-two-sum/solution.c) is a **standalone, self-contained C program** complete with data structure helpers (`struct ListNode`, `struct TreeNode`), complexity analysis, and a `main()` test runner that executes the official basic LeetCode example test cases.

---

## Building & Running Solutions

You can build and run all 26 solutions or any individual problem from `/Users/sanketdave/sourcecode/leetcode`:

```bash
# Compile and run all 26 LeetCode solutions and basic example test suites
make test

# Compile all 26 solution binaries without running them
make

# Compile and run a single problem by number (e.g., ID=1 or ID=0001)
make run ID=1
make run ID=0206

# Or compile and run by problem directory name
make 0001-two-sum
make 1114-print-in-order

# Clean all compiled binaries
make clean
```

- **From VS Code**: Open any `leetcode/<id>-<slug>/solution.c` file and press **`Cmd+Shift+B`** to compile it directly or **`F5`** to debug its `main()` test cases in `lldb`.

---

## Solved Problems Index (26 Problems)

| # | Problem | Difficulty | Language | Last Accepted | Solution & Test Harness |
| :--- | :--- | :--- | :--- | :--- | :--- |
| 1 | [Two Sum](https://leetcode.com/problems/two-sum/) | Easy | C | `2022-10-21` | [`solution.c`](./0001-two-sum/solution.c) |
| 2 | [Add Two Numbers](https://leetcode.com/problems/add-two-numbers/) | Medium | C | `2026-07-01` | [`solution.c`](./0002-add-two-numbers/solution.c) |
| 3 | [Longest Substring Without Repeating Characters](https://leetcode.com/problems/longest-substring-without-repeating-characters/) | Medium | C | `2022-10-24` | [`solution.c`](./0003-longest-substring-without-repeating-characters/solution.c) |
| 7 | [Reverse Integer](https://leetcode.com/problems/reverse-integer/) | Medium | C | `2017-01-19` | [`solution.c`](./0007-reverse-integer/solution.c) |
| 26 | [Remove Duplicates from Sorted Array](https://leetcode.com/problems/remove-duplicates-from-sorted-array/) | Easy | C | `2024-10-06` | [`solution.c`](./0026-remove-duplicates-from-sorted-array/solution.c) |
| 28 | [Find the Index of the First Occurrence in a String](https://leetcode.com/problems/find-the-index-of-the-first-occurrence-in-a-string/) | Easy | C | `2017-06-04` | [`solution.c`](./0028-find-the-index-of-the-first-occurrence-in-a-string/solution.c) |
| 53 | [Maximum Subarray](https://leetcode.com/problems/maximum-subarray/) | Medium | C | `2017-01-19` | [`solution.c`](./0053-maximum-subarray/solution.c) |
| 83 | [Remove Duplicates from Sorted List](https://leetcode.com/problems/remove-duplicates-from-sorted-list/) | Easy | C | `2026-07-07` | [`solution.c`](./0083-remove-duplicates-from-sorted-list/solution.c) |
| 92 | [Reverse Linked List II](https://leetcode.com/problems/reverse-linked-list-ii/) | Medium | C | `2017-05-16` | [`solution.c`](./0092-reverse-linked-list-ii/solution.c) |
| 98 | [Validate Binary Search Tree](https://leetcode.com/problems/validate-binary-search-tree/) | Medium | C | `2017-01-22` | [`solution.c`](./0098-validate-binary-search-tree/solution.c) |
| 104 | [Maximum Depth of Binary Tree](https://leetcode.com/problems/maximum-depth-of-binary-tree/) | Easy | C | `2017-01-23` | [`solution.c`](./0104-maximum-depth-of-binary-tree/solution.c) |
| 112 | [Path Sum](https://leetcode.com/problems/path-sum/) | Easy | C | `2017-01-20` | [`solution.c`](./0112-path-sum/solution.c) |
| 190 | [Reverse Bits](https://leetcode.com/problems/reverse-bits/) | Easy | C | `2016-05-31` | [`solution.c`](./0190-reverse-bits/solution.c) |
| 191 | [Number of 1 Bits](https://leetcode.com/problems/number-of-1-bits/) | Easy | C | `2017-01-18` | [`solution.c`](./0191-number-of-1-bits/solution.c) |
| 206 | [Reverse Linked List](https://leetcode.com/problems/reverse-linked-list/) | Easy | C | `2017-05-16` | [`solution.c`](./0206-reverse-linked-list/solution.c) |
| 226 | [Invert Binary Tree](https://leetcode.com/problems/invert-binary-tree/) | Easy | C | `2017-06-04` | [`solution.c`](./0226-invert-binary-tree/solution.c) |
| 235 | [Lowest Common Ancestor of a Binary Search Tree](https://leetcode.com/problems/lowest-common-ancestor-of-a-binary-search-tree/) | Medium | C | `2017-01-23` | [`solution.c`](./0235-lowest-common-ancestor-of-a-binary-search-tree/solution.c) |
| 283 | [Move Zeroes](https://leetcode.com/problems/move-zeroes/) | Easy | C | `2024-10-04` | [`solution.c`](./0283-move-zeroes/solution.c) |
| 328 | [Odd Even Linked List](https://leetcode.com/problems/odd-even-linked-list/) | Medium | C | `2017-01-19` | [`solution.c`](./0328-odd-even-linked-list/solution.c) |
| 338 | [Counting Bits](https://leetcode.com/problems/counting-bits/) | Easy | C | `2017-01-18` | [`solution.c`](./0338-counting-bits/solution.c) |
| 346 | [Moving Average from Data Stream](https://leetcode.com/problems/moving-average-from-data-stream/) | Easy | C | `2024-10-05` | [`solution.c`](./0346-moving-average-from-data-stream/solution.c) |
| 485 | [Max Consecutive Ones](https://leetcode.com/problems/max-consecutive-ones/) | Easy | C | `2017-01-23` | [`solution.c`](./0485-max-consecutive-ones/solution.c) |
| 693 | [Binary Number with Alternating Bits](https://leetcode.com/problems/binary-number-with-alternating-bits/) | Easy | C | `2026-09-30` | [`solution.c`](./0693-binary-number-with-alternating-bits/solution.c) |
| 1114 | [Print in Order](https://leetcode.com/problems/print-in-order/) | Easy | C | `2026-09-30` | [`solution.c`](./1114-print-in-order/solution.c) |
| 1115 | [Print FooBar Alternately](https://leetcode.com/problems/print-foobar-alternately/) | Medium | C | `2026-09-30` | [`solution.c`](./1115-print-foobar-alternately/solution.c) |
| 1762 | [Buildings With an Ocean View](https://leetcode.com/problems/buildings-with-an-ocean-view/) | Medium | C | `2024-10-06` | [`solution.c`](./1762-buildings-with-an-ocean-view/solution.c) |

---

## Syncing New Submissions

To pull newly accepted submissions from LeetCode and commit each solution with its original submission timestamp:

```bash
# Uses LEETCODE_SESSION env var or ~/.gemini/config/.leetcode_session
make sync
```
