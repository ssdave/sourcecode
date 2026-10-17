/* ============================================================================
 * LeetCode #1114: Print in Order (Easy)
 * URL:      https://leetcode.com/problems/print-in-order/
 * Language: C (pthreads)
 *
 * What is being built / Approach:
 *   Synchronizes three concurrent threads invoking `first()`, `second()`, and
 *   `third()` in arbitrary order using a POSIX mutex (`pthread_mutex_t`) and
 *   condition variable (`pthread_cond_t`) gated on state counter `val`.
 *   Time Complexity: O(1) | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: threads = [1, 2, 3] -> "firstsecondthird"
 *   - Example 2: threads = [1, 3, 2] -> "firstsecondthird"
 * ============================================================================ */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char g_output[64];

void printFirst(void)
{
  strcat(g_output, "first");
}

void printSecond(void)
{
  strcat(g_output, "second");
}

void printThird(void)
{
  strcat(g_output, "third");
}

typedef struct
{
  pthread_cond_t cond;
  pthread_mutex_t lock;
  int val;
} Foo;

Foo *fooCreate(void)
{
  Foo *obj = (Foo *)malloc(sizeof(Foo));

  pthread_cond_init(&obj->cond, NULL);
  pthread_mutex_init(&obj->lock, NULL);
  obj->val = 1;
  return obj;
}

void first(Foo *obj)
{
  pthread_mutex_lock(&obj->lock);
  while(obj->val != 1)
  {
    pthread_cond_wait(&obj->cond, &obj->lock);
  }

  printFirst();
  obj->val++;
  pthread_cond_broadcast(&obj->cond);
  pthread_mutex_unlock(&obj->lock);
}

void second(Foo *obj)
{
  pthread_mutex_lock(&obj->lock);
  while(obj->val != 2)
  {
    pthread_cond_wait(&obj->cond, &obj->lock);
  }

  printSecond();

  obj->val++;
  pthread_cond_broadcast(&obj->cond);
  pthread_mutex_unlock(&obj->lock);
}

void third(Foo *obj)
{
  pthread_mutex_lock(&obj->lock);
  while(obj->val != 3)
  {
    pthread_cond_wait(&obj->cond, &obj->lock);
  }
  printThird();
  pthread_mutex_unlock(&obj->lock);
}

void fooFree(Foo *obj)
{
  pthread_cond_destroy(&obj->cond);
  pthread_mutex_destroy(&obj->lock);
  free(obj);
}

static void *runFirst(void *arg)
{
  first((Foo *)arg);
  return NULL;
}

static void *runSecond(void *arg)
{
  second((Foo *)arg);
  return NULL;
}

static void *runThird(void *arg)
{
  third((Foo *)arg);
  return NULL;
}

int main(void)
{
  printf("=== LeetCode #1114: Print in Order ===\n");

  /* Example 1: [1, 2, 3] */
  g_output[0] = '\0';
  Foo *foo1 = fooCreate();
  pthread_t t1, t2, t3;
  pthread_create(&t1, NULL, runFirst, foo1);
  pthread_create(&t2, NULL, runSecond, foo1);
  pthread_create(&t3, NULL, runThird, foo1);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  pthread_join(t3, NULL);
  printf("Example 1: order=[1,2,3] -> \"%s\"\n", g_output);
  assert(strcmp(g_output, "firstsecondthird") == 0);
  fooFree(foo1);

  /* Example 2: [1, 3, 2] */
  g_output[0] = '\0';
  Foo *foo2 = fooCreate();
  pthread_create(&t1, NULL, runFirst, foo2);
  pthread_create(&t3, NULL, runThird, foo2);
  pthread_create(&t2, NULL, runSecond, foo2);
  pthread_join(t1, NULL);
  pthread_join(t3, NULL);
  pthread_join(t2, NULL);
  printf("Example 2: order=[1,3,2] -> \"%s\"\n", g_output);
  assert(strcmp(g_output, "firstsecondthird") == 0);
  fooFree(foo2);

  printf("All tests passed!\n");
  return 0;
}
