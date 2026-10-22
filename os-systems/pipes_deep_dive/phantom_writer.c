/**
 * @file phantom_writer.c
 * @brief Demonstrates and resolves the UNIX Pipe "Phantom Writer" Deadlock.
 *
 * Concepts Demonstrated:
 * 1. Kernel reference counting on pipe file descriptors across fork().
 * 2. Why read() blocks indefinitely if any process in the system retains an open write descriptor.
 * 3. The correct descriptor lifecycle: immediate closure of unused descriptors post-fork.
 *
 * Problem Description:
 * In a parent-child pipeline where the child writes data and the parent reads until EOF:
 * When fork() is called, the child inherits duplicate descriptors for both ends of the pipe.
 * If the parent fails to close its own copy of the write descriptor (WRITE_FD), the pipe's
 * writer reference count will never drop to zero—even after the child completes and exits!
 * Consequently, the parent's read() loop will block indefinitely (starving on EOF) waiting
 * for more data that will never arrive. This condition is known as a "Phantom Writer" deadlock.
 *
 * Academic Attribution:
 * Exercise specification based on operating systems concurrency and IPC principles
 * (Rose-Hulman CSSE 332 & OSTEP Chapter 5). Retained strictly for independent educational
 * self-study.
 */

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ_FD 0
#define WRITE_FD 1

/**
 * Executes a child worker pipeline task and safely drains its output into out_buffer.
 *
 * Contract & Requirements:
 * - Spawns a child process to transmit `message` through a UNIX pipe.
 * - Parent drains the pipe output into `out_buffer` (up to `out_capacity - 1` bytes) until EOF.
 * - Guarantees safe null-termination of `out_buffer`.
 * - Prevents read hangs/deadlocks by managing file descriptor lifecycles properly.
 * - Reaps child process cleanly without leaving zombies.
 *
 * @param message The string payload for the child to write into the pipe.
 * @param out_buffer Destination buffer where parent collects the child's output.
 * @param out_capacity Maximum capacity of out_buffer.
 * @return Total bytes collected by parent, or -1 on error.
 */
ssize_t execute_child_writer_pipeline(const char *message, char *out_buffer, size_t out_capacity)
{
  /* TODO: Author Implementation */

  return -1;
}

static void timeout_handler(int sig)
{
  (void)sig;
  fprintf(stderr,
          "\n[FATAL] Test timed out! Deadlock detected: Phantom Writer unclosed WRITE_FD!\n");
  exit(2);
}

int main(void)
{
  printf("=== Test Suite: Phantom Writer Deadlock Prevention ===\n");

  // Install timeout watchdog to catch deadlocks if write descriptor is not closed
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = timeout_handler;
  sigaction(SIGALRM, &sa, NULL);

  const char *test_msg = "Worker status: Task completed successfully with zero leaks.";
  char buffer[256];
  memset(buffer, 0, sizeof(buffer));

  // Arm 3-second watchdog timer
  alarm(3);

  ssize_t bytes_read = execute_child_writer_pipeline(test_msg, buffer, sizeof(buffer));

  // Disarm watchdog timer
  alarm(0);

  if(bytes_read == -1)
  {
    printf("[Exercise Scaffold] execute_child_writer_pipeline() returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    return 0;
  }

  printf("[Verification] Bytes read: %zd\n", bytes_read);
  printf("[Verification] Payload: \"%s\"\n", buffer);

  assert(bytes_read == (ssize_t)strlen(test_msg));
  assert(strcmp(buffer, test_msg) == 0);

  printf("=== PASS: Phantom Writer handled cleanly with EOF and zero deadlocks! ===\n");
  return 0;
}
