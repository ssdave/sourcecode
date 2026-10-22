/**
 * @file broken_pipe.c
 * @brief Demonstrates Broken Pipe mechanics, SIGPIPE signal handling, and EPIPE recovery.
 *
 * Concepts Demonstrated:
 * 1. What occurs when write() is invoked on a pipe whose read ends are ALL closed.
 * 2. Kernel default behavior: Delivery of SIGPIPE (which terminates the process by default!).
 * 3. Defensive systems programming: Catching or ignoring SIGPIPE to handle errno == EPIPE
 * gracefully.
 *
 * Problem Description:
 * In robust systems programming (e.g. shells, web servers, IPC pipelines), a writer process
 * must never crash simply because a downstream consumer process terminated or closed its reader
 * end prematurely. When all read ends of a pipe are closed, the kernel raises the SIGPIPE signal
 * to the writer. By default, SIGPIPE abruptly terminates the process without producing a core dump.
 * Systems programs must install a custom signal handler or configure signal masks so that write()
 * fails safely with -1 and sets errno to EPIPE, allowing the application to recover.
 *
 * Academic Attribution:
 * Exercise specification based on POSIX IPC signal mechanics (Rose-Hulman CSSE 332 & Stevens APUE).
 * Retained strictly for independent educational self-study.
 */

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define READ_FD 0
#define WRITE_FD 1

static volatile sig_atomic_t g_sigpipe_count = 0;

void sigpipe_counter_handler(int sig)
{
  (void)sig;
  g_sigpipe_count++;
}

/**
 * Installs a custom signal handler for SIGPIPE.
 *
 * Contract:
 * - Registers handler for SIGPIPE.
 * - Returns 0 on success, or -1 on error.
 *
 * @param handler Function pointer to the signal handling function.
 * @return 0 on success, -1 on failure.
 */
int install_sigpipe_handler(void (*handler)(int))
{
  (void)handler;
  /* TODO: Author Implementation */
  return -1;
}

/**
 * Safely writes data to a pipe file descriptor and detects broken pipe conditions.
 *
 * Contract & Requirements:
 * - Writes count bytes from data into write_fd.
 * - Distinguishes EPIPE (broken pipe) from normal write completions and other errors.
 * - Sets *out_is_broken_pipe appropriately if the pointer is non-NULL.
 * - Returns number of bytes written, or -1 on error.
 *
 * @param write_fd The pipe write end descriptor.
 * @param data Pointer to the buffer to write.
 * @param count Number of bytes to write.
 * @param out_is_broken_pipe Output flag set to 1 if write failed due to EPIPE, or 0 otherwise.
 * @return Number of bytes written, or -1 on error.
 */
ssize_t safe_pipe_write(int write_fd, const void *data, size_t count, int *out_is_broken_pipe)
{
  (void)write_fd;
  (void)data;
  (void)count;
  (void)out_is_broken_pipe;
  /* TODO: Author Implementation */
  return -1;
}

int main(void)
{
  printf("=== Test Suite: Broken Pipe (SIGPIPE & EPIPE) Handling ===\n");

  int pipe_fds[2];

  if(pipe(pipe_fds) < 0)
  {
    perror("pipe failed");
    return 1;
  }

  // 1. Install custom SIGPIPE handler
  int install_rc = install_sigpipe_handler(sigpipe_counter_handler);

  if(install_rc == -1)
  {
    printf("[Exercise Scaffold] install_sigpipe_handler() returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    close(pipe_fds[READ_FD]);
    close(pipe_fds[WRITE_FD]);
    return 0;
  }

  // 2. Normal write test while reader is open
  int is_broken = 0;
  const char *normal_msg = "Normal packet";
  ssize_t bytes_written =
      safe_pipe_write(pipe_fds[WRITE_FD], normal_msg, strlen(normal_msg), &is_broken);

  if(bytes_written == -1 && is_broken == 0)
  {
    printf("[Exercise Scaffold] safe_pipe_write() returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    close(pipe_fds[READ_FD]);
    close(pipe_fds[WRITE_FD]);
    return 0;
  }

  assert(bytes_written == (ssize_t)strlen(normal_msg));
  assert(is_broken == 0);
  assert(g_sigpipe_count == 0);
  printf("[Verification] Normal write succeeded (%zd bytes written).\n", bytes_written);

  // Drain the normal message from the pipe
  char drain_buf[64];
  read(pipe_fds[READ_FD], drain_buf, sizeof(drain_buf));

  // 3. Broken pipe test: explicitly close reader end!
  printf("[Verification] Closing READ_FD (%d). Zero readers now exist.\n", pipe_fds[READ_FD]);
  close(pipe_fds[READ_FD]);

  const char *broken_msg = "This write must trigger SIGPIPE and fail with EPIPE";
  bytes_written = safe_pipe_write(pipe_fds[WRITE_FD], broken_msg, strlen(broken_msg), &is_broken);

  assert(bytes_written == -1);
  assert(is_broken == 1);
  assert(g_sigpipe_count == 1);

  printf("[Verification] Broken pipe handled cleanly: write() returned -1, is_broken=1, SIGPIPE "
         "caught!\n");

  close(pipe_fds[WRITE_FD]);

  printf("=== PASS: Broken pipe resilience verified successfully! ===\n");
  return 0;
}
