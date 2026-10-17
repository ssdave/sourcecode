/**
 * @file process_test_runner.c
 * @brief Multi-process parallelized C unit testing framework.
 *
 * Parallelizes test execution using POSIX process management primitives (fork, waitpid,
 * pipe, kill/signals). Each test runs in an isolated child process to guard against
 * faults (crashes / timeouts) without terminating the test runner.
 *
 * Completed Milestones (All 6 Steps):
 * - Step 1: Run all tests in parallel using fork() and waitpid() reaping.
 * - Step 2: Exit status translation: child returns 0 on pass, 1 on fail; parent inspects
 * WEXITSTATUS.
 * - Step 3: Shared setup optimization: run setup() once in parent before fork() using
 * copy-on-write.
 * - Step 4: Fault isolation: parent catches crash signals (e.g., SIGSEGV) via WIFSIGNALED.
 * - Step 5: Execution timeout: alarm(3) in child terminates hung tests with SIGALRM.
 * - Step 6: IPC error reporting: inter-process pipes (pipe()) stream failure details to parent.
 *
 * Academic Attribution:
 * Derived from Rose-Hulman Institute of Technology (RHIT) CSSE 332: Operating Systems
 * Lab 03 -- Process Lab (Course URL:
 * https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/processlab/). Faculty authors: Mohammad
 * Noureddine, Michael Hewner, Sid Stamm, and CSSE 332 contributors. Retained strictly for
 * independent educational self-study.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define simple_assert(message, test)                                                               \
  do                                                                                               \
  {                                                                                                \
    if(!(test))                                                                                    \
    {                                                                                              \
      return message;                                                                              \
    }                                                                                              \
  } while(0)

#define TEST_PASSED NULL
#define DATA_SIZE 100
#define INITIAL_VALUE 77
#define MAX_TESTS 10
#define READ_FD 0
#define WRITE_FD 1

/* Array of function pointers storing registered test cases. */
char *(*test_funcs[MAX_TESTS])();

/* Total number of tests registered */
static int num_tests = 0;

/* Shared data initialized by setup() */
static int data[DATA_SIZE][DATA_SIZE];

/**
 * Register a test function into the suite.
 */
void add_test(char *(*test_func)())
{
  if(num_tests == MAX_TESTS)
  {
    printf("exceeded max possible tests\n");
    exit(1);
  }
  test_funcs[num_tests] = test_func;
  num_tests++;
}

/**
 * Setup function run to initialize data before test execution.
 *
 * Nuance - Copy-on-Write (COW):
 * In Step 3, calling setup() once in the parent process prior to fork() avoids
 * repeating expensive initialization (e.g. 3-second sleep) across N tests.
 * Thanks to OS virtual memory copy-on-write semantics, child processes inherit
 * read-only mappings to the parent's initialized pages. If a child mutates
 * the data array, the MMU triggers a page fault and duplicates only the modified
 * page into child-private memory, ensuring zero cross-test state leakage.
 */
void setup(void)
{
  printf("starting setup\n");
  fflush(stdout);
  for(int i = 0; i < DATA_SIZE; i++)
  {
    for(int j = 0; j < DATA_SIZE; j++)
    {
      data[i][j] = INITIAL_VALUE;
    }
  }

  /* Simulate complex setup operations that take time. */
  usleep(3000000);
}

/**
 * Run all registered tests in parallel child processes and collect results.
 */
void run_all_tests(void)
{
  pid_t pids[MAX_TESTS];
  int pipes[MAX_TESTS][2];
  pid_t terminated_pid;
  int status = -1;

  /* Step 3: Run expensive setup once in parent; children inherit state via COW. */
  setup();

  /* Step 1: Dispatch each test function into its own isolated child process. */
  for(int iter = 0; iter < num_tests; iter++)
  {
    if(test_funcs[iter] != NULL)
    {
      /* Step 6: Create dedicated unidirectional pipe for test failure diagnostics. */
      pipe(pipes[iter]);
      pids[iter] = fork();
      if(pids[iter] == 0)
      {
        /*
         * Child Process:
         * Pipe lifecycle nuance: Close unused read end immediately.
         */
        close(pipes[iter][READ_FD]);

        /*
         * Step 5 - Execution Timeout:
         * Schedule a SIGALRM signal to be delivered in 3 seconds. If the test
         * enters an infinite loop or hangs, the OS terminates the child with
         * SIGALRM (uncaught signal), preventing the entire test harness from stalling.
         */
        alarm(3);

        /* Execute the registered test function. */
        char *result = (*test_funcs[iter])();

        /*
         * Step 2 - Exit Status Translation:
         * Standardize outcome: return 0 for PASS, 1 for FAIL.
         * In Step 6, stream the failure assertion message across the pipe before exiting.
         */
        if(result == TEST_PASSED)
        {
          exit(0);
        }
        else
        {
          write(pipes[iter][WRITE_FD], result, strlen(result));
          close(pipes[iter][WRITE_FD]);
          exit(1);
        }
      }
      if(pids[iter] > 0)
      {
        /*
         * Parent Process:
         * Pipe lifecycle nuance: Close write end in parent immediately.
         * If the parent leaves WRITE_FD open, the pipe's write reference count
         * never drops to 0, which would cause subsequent read() calls to block
         * indefinitely instead of seeing EOF when the child terminates.
         */
        close(pipes[iter][WRITE_FD]);
      }
      // AI Review Feedback: Consider handling fork() failures (pids[iter] < 0). If fork() returns
      // -1 (e.g. system PID or resource limit reached), both pipe file descriptors (pipes[iter][0]
      // and pipes[iter][1]) leak in the parent process, and pids[iter] remains -1 without error
      // diagnostics.
    }
  }

  /*
   * Step 1 & 4 - Asynchronous Process Reaping:
   * waitpid(-1, &status, 0) blocks until ANY child finishes (reaping whichever child
   * completes first in non-deterministic arrival order). Continues until all children
   * have been reaped and waitpid returns -1 (ECHILD).
   */
  while((terminated_pid = waitpid(-1, &status, 0)) > 0)
  {
    /*
     * POSIX Status Nuance:
     * Must check WIFEXITED(status) BEFORE inspecting WEXITSTATUS(status).
     * WIFEXITED evaluates true only if the process called exit() or returned from main().
     */
    if(WIFEXITED(status))
    {
      if(WEXITSTATUS(status) == 0)
      {
        printf("Test Done for %d with PASS\n", terminated_pid);
      }
      else
      {
        /* Match terminated child PID to its pipe index to retrieve error description. */
        for(int iter = 0; iter < num_tests; iter++)
        {
          if(pids[iter] == terminated_pid)
          {
            char buf[256];
            /*
             * Stream Framing & NULL-Termination Nuance:
             * read() returns a raw stream of octets and does NOT append a null terminator.
             * We must explicitly null-terminate at buf[bytes] before passing to printf("%s").
             */
            ssize_t bytes = read(pipes[iter][READ_FD], buf, sizeof(buf) - 1);
            // AI Review Feedback: Check 'if(bytes > 0)' instead of 'if(bytes)'. If read() fails or
            // is interrupted and returns -1, evaluating 'if(bytes)' as true causes an out-of-bounds
            // write at buf[-1]. Setting buf[bytes > 0 ? bytes : 0] = '\0' guarantees safe
            // null-termination.
            if(bytes)
            {
              buf[bytes] = '\0';
            }
            printf("Test Done for %d with FAIL. Reason: %s\n", terminated_pid, buf);
            close(pipes[iter][READ_FD]);
          }
        }
      }
    }

    /*
     * Step 4 & 5 - Signal vs Normal Exit:
     * WIFSIGNALED is true if the child was terminated by an unhandled signal.
     * WTERMSIG extracts the signal number to distinguish between:
     * - SIGALRM: Timed out from alarm(3)
     * - Hardware faults (e.g. SIGSEGV / SIGBUS from dereferencing NULL)
     */
    if(WIFSIGNALED(status))
    {
      if(WTERMSIG(status) == SIGALRM)
      {
        printf("Test Timed Out for %d\n", terminated_pid);
      }
      else
      {
        printf("Test crashed for %d with signal %s\n", terminated_pid, strsignal(WTERMSIG(status)));
      }
    }

    // AI Review Feedback: Potential file descriptor leak on PASS, crash, or timeout. The parent
    // process only closes pipes[iter][READ_FD] when WIFEXITED is true and WEXITSTATUS is non-zero.
    // For test cases that pass (exit status 0) or terminate due to signals (SIGSEGV / SIGALRM),
    // pipes[iter][READ_FD] is never closed, leaving open file descriptors in the parent until
    // process exit.
  }
}

/**
 * Test 1: Verifies initial shared state and tests copy-on-write memory isolation.
 * Mutates data elements to 1; verified locally within this child process.
 */
char *test1(void)
{
  printf("starting test 1\n");
  for(int i = 0; i < DATA_SIZE; i++)
  {
    for(int j = 0; j < DATA_SIZE; j++)
    {
      simple_assert("test 1 data not initialized properly", data[i][j] == INITIAL_VALUE);
    }
  }

  for(int i = 0; i < DATA_SIZE; i++)
  {
    for(int j = 0; j < DATA_SIZE; j++)
    {
      data[i][j] = 1;
    }
  }

  for(int i = 0; i < DATA_SIZE; i++)
  {
    for(int j = 0; j < DATA_SIZE; j++)
    {
      simple_assert("test 1 data not set properly", data[i][j] == 1);
    }
  }
  printf("ending test 1\n");
  return TEST_PASSED;
}

/**
 * Test 2: Verifies initial state independently of test1.
 * Mutates data elements to 2; proves test1's writes did not bleed across process boundaries.
 */
char *test2(void)
{
  printf("starting test 2\n");
  for(int i = 0; i < DATA_SIZE; i++)
  {
    for(int j = 0; j < DATA_SIZE; j++)
    {
      simple_assert("test 2 data not initialized properly", data[i][j] == INITIAL_VALUE);
    }
  }

  for(int i = 0; i < DATA_SIZE; i++)
  {
    for(int j = 0; j < DATA_SIZE; j++)
    {
      data[i][j] = 2;
    }
  }

  for(int i = 0; i < DATA_SIZE; i++)
  {
    for(int j = 0; j < DATA_SIZE; j++)
    {
      simple_assert("test 2 data not set properly", data[i][j] == 2);
    }
  }

  printf("ending test 2\n");
  return TEST_PASSED;
}

/**
 * Test 3: Tests assertion failure reporting via exit status and IPC pipe.
 * Always triggers simple_assert failure, testing non-zero exit code and pipe error streaming.
 */
char *test3(void)
{
  printf("starting test 3\n");

  simple_assert("test 3 always fails", 1 == 2);

  printf("ending test 3\n");
  return TEST_PASSED;
}

/**
 * Test 4: Tests fault isolation against crashes (hardware signals).
 * Intentionally dereferences a NULL pointer to cause SIGSEGV; verifies test runner survival.
 */
char *test4(void)
{
  printf("starting test 4\n");
  fflush(stdout);

  int *val = NULL;
  printf("data at val is %d\n", *val);

  printf("ending test 4\n");
  return TEST_PASSED;
}

/**
 * Test 5: Tests execution timeout enforcement.
 * Enters an intentional infinite loop; verified that alarm(3) triggers SIGALRM and runner reaps it.
 */
char *test5(void)
{
  printf("starting test 5\n");
  fflush(stdout);

  while(1)
  {
  }

  printf("ending test 5\n");
  return TEST_PASSED;
}

int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  add_test(test1);
  add_test(test2);
  add_test(test3);
  add_test(test4);
  add_test(test5);

  run_all_tests();
  return 0;
}
