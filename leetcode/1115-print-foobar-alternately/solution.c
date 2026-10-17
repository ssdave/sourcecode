/* ============================================================================
 * LeetCode #1115: Print FooBar Alternately (Medium)
 * URL:      https://leetcode.com/problems/print-foobar-alternately/
 * Language: C (pthreads)
 *
 * What is being built / Approach:
 *   Alternates execution between two threads calling `foo()` and `bar()` `n`
 *   times using a mutex (`pthread_mutex_t`), condition variable (`pthread_cond_t`),
 *   and parity counter `num % 2`.
 *   Time Complexity: O(n) | Space Complexity: O(1)
 *
 * What is being tested (Basic LeetCode Examples):
 *   - Example 1: n = 1 -> "foobar"
 *   - Example 2: n = 2 -> "foobarfoobar"
 * ============================================================================ */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char g_output[128];

void printFoo(void) { strcat(g_output, "foo"); }

void printBar(void) { strcat(g_output, "bar"); }

typedef struct
{
  int n;
  int num;
  pthread_cond_t cond;
  pthread_mutex_t lock;
} FooBar;

FooBar *fooBarCreate(int n)
{
  FooBar *obj = (FooBar *)malloc(sizeof(FooBar));
  obj->n = n;
  obj->num = 0;
  pthread_cond_init(&obj->cond, NULL);
  pthread_mutex_init(&obj->lock, NULL);
  return obj;
}

void foo(FooBar *obj)
{
  for(int i = 0; i < obj->n; i++)
  {
    pthread_mutex_lock(&obj->lock);
    while((obj->num % 2) != 0)
    {
      pthread_cond_wait(&obj->cond, &obj->lock);
    }
    printFoo();
    obj->num++;
    pthread_cond_signal(&obj->cond);
    pthread_mutex_unlock(&obj->lock);
  }
}

void bar(FooBar *obj)
{
  for(int i = 0; i < obj->n; i++)
  {
    pthread_mutex_lock(&obj->lock);
    while((obj->num % 2) != 1)
    {
      pthread_cond_wait(&obj->cond, &obj->lock);
    }

    printBar();
    obj->num++;
    pthread_cond_signal(&obj->cond);
    pthread_mutex_unlock(&obj->lock);
  }
}

void fooBarFree(FooBar *obj)
{
  pthread_cond_destroy(&obj->cond);
  pthread_mutex_destroy(&obj->lock);
  free(obj);
}

static void *runFoo(void *arg)
{
  foo((FooBar *)arg);
  return NULL;
}

static void *runBar(void *arg)
{
  bar((FooBar *)arg);
  return NULL;
}

int main(void)
{
  printf("=== LeetCode #1115: Print FooBar Alternately ===\n");

  g_output[0] = '\0';
  FooBar *fb1 = fooBarCreate(1);
  pthread_t t1, t2;
  pthread_create(&t1, NULL, runFoo, fb1);
  pthread_create(&t2, NULL, runBar, fb1);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  printf("Example 1: n=1 -> \"%s\"\n", g_output);
  assert(strcmp(g_output, "foobar") == 0);
  fooBarFree(fb1);

  g_output[0] = '\0';
  FooBar *fb2 = fooBarCreate(2);
  pthread_create(&t2, NULL, runBar, fb2);
  pthread_create(&t1, NULL, runFoo, fb2);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  printf("Example 2: n=2 -> \"%s\"\n", g_output);
  assert(strcmp(g_output, "foobarfoobar") == 0);
  fooBarFree(fb2);

  printf("All tests passed!\n");
  return 0;
}
