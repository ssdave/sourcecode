/* ============================================================================
 * File: dining_philosophers.c
 *
 * Attribution & Copyright:
 *   Originates from Edsger W. Dijkstra (1965, 1971) "Hierarchical Ordering of
 *   Sequential Processes", EWD310, Edward G. Coffman Jr., Michael J. Elphick,
 *   Arie Shoshani (1971) "System Deadlocks", ACM Computing Surveys, and Remzi
 *   Arpaci-Dusseau & Andrea Arpaci-Dusseau (OSTEP Chapter 32: Concurrency Bugs).
 *   Completed independently for Systems & Concurrency self-study.
 *
 * Systems Engineering & Concurrency Architecture:
 *   1. Coffman's Four Conditions for Deadlock:
 *      A deadlock can occur IF AND ONLY IF all four of the following hold:
 *        a) Mutual Exclusion: Resources cannot be shared (fork mutexes).
 *        b) Hold and Wait: A thread holds allocated resources while waiting for more.
 *        c) No Preemption: Resources cannot be forcibly seized from holders.
 *        d) Circular Wait: An unbroken circular chain of threads exists where each
 *           thread holds a resource needed by the next thread in the cycle.
 *
 *   2. Mathematical Proof of Deadlock Freedom via Resource Hierarchy:
 *      - Assign a strict total ordering to all resources: O(F_k) = k for k in [0, N-1].
 *      - Protocol Rule: Any thread requiring multiple resources must acquire them in
 *        strictly increasing order of their indices: min(fork_A, fork_B) before
 *        max(fork_A, fork_B).
 *      - Proof by Contradiction:
 *        Assume a deadlock state exists. Then there must exist a circular wait chain
 *        P_0 -> P_1 -> ... -> P_{k-1} -> P_0, where thread P_i holds fork F_{h_i} and
 *        is blocked waiting for fork F_{w_i} held by P_{(i+1)%k}.
 *        According to our total ordering rule, since P_i already holds F_{h_i} and is
 *        requesting F_{w_i}, it must be that O(F_{h_i}) < O(F_{w_i}).
 *        Propagating this relation around the cycle:
 *          O(F_{h_0}) < O(F_{w_0}) = O(F_{h_1}) < O(F_{w_1}) < ... < O(F_{h_0})
 *        This yields the strict inequality O(F_{h_0}) < O(F_{h_0}), which is an
 *        irreflexive contradiction.
 *        Hence, circular wait is mathematically impossible under total lock ordering.
 *
 * What is being built:
 *   A deadlock-free solution to Dijkstra's Dining Philosophers problem for
 *   5 philosophers and 5 forks (mutexes) enforcing total lock ordering.
 *
 * What is being tested:
 *   A high-throughput stress test: 5 concurrent philosophers each completing 20
 *   meals under randomized think/eat jitter. An active POSIX watchdog timer
 *   (5 seconds) catches any deadlock, while meal counters verify zero starvation.
 * ============================================================================ */

#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5
#define MEALS_COUNT 20
#define WATCHDOG_TIMEOUT_SECS 5

/* Global fork mutexes representing the shared resources at the table */
pthread_mutex_t forks[NUM_PHILOSOPHERS];

/* Set to 1 once author implementation is complete */
static int is_implemented = 0;

/**
 * Acquires both forks required for the specified philosopher to eat.
 *
 * @param philosopher_id Index of the philosopher (0 to NUM_PHILOSOPHERS - 1).
 */
void pickup_forks(int philosopher_id)
{
  (void)philosopher_id;
  /* TODO: Author Implementation */
}

/**
 * Releases both forks held by the specified philosopher.
 *
 * @param philosopher_id Index of the philosopher (0 to NUM_PHILOSOPHERS - 1).
 */
void putdown_forks(int philosopher_id)
{
  (void)philosopher_id;
  /* TODO: Author Implementation */
}

/* ============================================================================
 * High-Throughput Stress Test Harness & Watchdog Engine
 * ============================================================================ */

static int meals_eaten[NUM_PHILOSOPHERS];
static int fork_in_use[NUM_PHILOSOPHERS];
static pthread_mutex_t audit_mutex = PTHREAD_MUTEX_INITIALIZER;

static void watchdog_timeout_handler(int sig)
{
  (void)sig;
  fprintf(stderr, "\n[FATAL] Watchdog timeout (%ds): Deadlock detected in dining_philosophers!\n",
          WATCHDOG_TIMEOUT_SECS);
  exit(1);
}

void *philosopher_thread(void *arg)
{
  long id = (long)arg;
  int left_fork = (int)id;
  int right_fork = ((int)id + 1) % NUM_PHILOSOPHERS;

  for(int round = 0; round < MEALS_COUNT; round++)
  {
    /* Thinking phase with randomized jitter */
    usleep((useconds_t)((rand() % 400) + 100));

    /* Acquire forks using total lock ordering */
    pickup_forks((int)id);

    /* Eating phase (verify mutual exclusion on forks) */
    pthread_mutex_lock(&audit_mutex);
    if(fork_in_use[left_fork] || fork_in_use[right_fork])
    {
      fprintf(stderr, "[FATAL] Fork conflict: Philosopher %ld eating while fork in use!\n", id);
      exit(1);
    }
    fork_in_use[left_fork] = 1;
    fork_in_use[right_fork] = 1;
    pthread_mutex_unlock(&audit_mutex);

    /* Simulated eating duration */
    usleep((useconds_t)((rand() % 300) + 100));

    pthread_mutex_lock(&audit_mutex);
    fork_in_use[left_fork] = 0;
    fork_in_use[right_fork] = 0;
    meals_eaten[id]++;
    pthread_mutex_unlock(&audit_mutex);

    /* Release forks */
    putdown_forks((int)id);
  }

  return NULL;
}

int main(void)
{
  printf("=== Dining Philosophers Stress Test (Dijkstra Resource Hierarchy) ===\n");
  printf("Config: %d philosophers, %d meals each (%d total meals), %d second watchdog\n",
         NUM_PHILOSOPHERS, MEALS_COUNT, NUM_PHILOSOPHERS * MEALS_COUNT, WATCHDOG_TIMEOUT_SECS);

  if(!is_implemented)
  {
    printf("[Exercise Scaffold] Awaiting author implementation for dining_philosophers.\n");
    return 0;
  }

  /* Arm watchdog timer */
  signal(SIGALRM, watchdog_timeout_handler);
  alarm(WATCHDOG_TIMEOUT_SECS);

  for(int i = 0; i < NUM_PHILOSOPHERS; i++)
  {
    pthread_mutex_init(&forks[i], NULL);
    meals_eaten[i] = 0;
    fork_in_use[i] = 0;
  }

  pthread_t philosophers[NUM_PHILOSOPHERS];
  for(long i = 0; i < NUM_PHILOSOPHERS; i++)
  {
    pthread_create(&philosophers[i], NULL, philosopher_thread, (void *)i);
  }

  for(int i = 0; i < NUM_PHILOSOPHERS; i++)
  {
    pthread_join(philosophers[i], NULL);
  }

  /* Disarm watchdog timer */
  alarm(0);

  for(int i = 0; i < NUM_PHILOSOPHERS; i++)
  {
    pthread_mutex_destroy(&forks[i]);
  }

  int total_meals = 0;
  int min_meals = MEALS_COUNT;
  for(int i = 0; i < NUM_PHILOSOPHERS; i++)
  {
    total_meals += meals_eaten[i];
    if(meals_eaten[i] < min_meals)
    {
      min_meals = meals_eaten[i];
    }
  }

  int expected_meals = NUM_PHILOSOPHERS * MEALS_COUNT;
  printf("Summary: total_meals=%d (expected=%d), min_meals_per_philosopher=%d\n", total_meals,
         expected_meals, min_meals);

  if(total_meals == expected_meals && min_meals == MEALS_COUNT)
  {
    printf("[PASS] All philosophers completed meals without deadlock or starvation.\n");
    return 0;
  }
  else
  {
    printf("[FAIL] Meal count discrepancy or starvation detected.\n");
    return 1;
  }
}
