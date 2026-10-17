/* ============================================================================
 * File: simpleshell.c
 *
 * Attribution & Copyright:
 *   Copyright 2016 Rose-Hulman Institute of Technology — CSSE 332: Operating Systems
 *   Based on idea from Jacobs University Bremen CAOS Lab:
 *     http://cnds.eecs.jacobs-university.de/courses/caoslab-2007/
 *   Course Schedule: https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/
 *   Lab Spec:        https://www.rose-hulman.edu/class/csse/csse332/2324b/labs/lab01/
 *   Lab Archive:     https://github.com/rhit-csse332/csse332-labs-archive
 *   Authors:         RHIT CSSE 332 Faculty (M. Noureddine, M. Hewner, S. Stamm,
 *                    R. Williamson, S. Mohan)
 *   Completed independently for Operating Systems self-study.
 *
 * What is being built:
 *   Interactive UNIX command shell loop that reads commands from stdin, splits
 *   the executable name and argument, handles built-in `quit`/`exit`, and uses
 *   `fork()`, `execlp()`, and `wait()` / `WEXITSTATUS()` to execute foreground
 *   and `BG`-prefixed background commands. Implements asynchronous `SIGCHLD`
 *   signal handling to reap terminated background watcher processes and prevent
 *   zombie process accumulation.
 * ============================================================================ */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void handle_sigchld(int ignored)
{
  // AI Review Feedback: Calling wait(NULL) once inside SIGCHLD has three subtle systems nuances:
  // 1. Signal Coalescing: POSIX signals do not queue. If multiple children exit concurrently,
  //    only one SIGCHLD is delivered; calling wait(NULL) once leaves remaining dead children as
  //    zombies. Use `while(waitpid(-1, NULL, WNOHANG) > 0);` to reap all pending terminated
  //    children.
  // 2. Double-Wait / Race with Foreground wait(&status): When a foreground child exits, SIGCHLD
  //    fires. If handle_sigchld runs first, wait(NULL) reaps the foreground child, causing the
  //    parent's foreground wait(&status) to return -1 (ECHILD). In production, waitpid() should
  //    target specific foreground PIDs (waitpid(child_pid, ...)), leaving WNOHANG for background
  //    reaping.
  // 3. errno Preservation: wait/waitpid can clobber errno. Signal handlers should save and
  //    restore errno (`int saved_errno = errno; ... errno = saved_errno;`) so interrupted syscalls
  //    do not see corrupted errno.
  wait(NULL);
}

int main()
{
  char command[82];
  char *parsed_command[2];
  /* Supports command line with at most two tokens (command and optional argument). */
  signal(SIGCHLD, handle_sigchld);
  while(1)
  {
    printf("SHELL%% ");
    // AI Review Feedback: fgets() return value is not checked for NULL (e.g. on EOF/Ctrl-D or
    // EINTR). If fgets() returns NULL or an empty line, command[strlen(command) - 1] can result in
    // out-of-bounds memory access or infinite loops on EOF.
    fgets(command, 82, stdin);
    command[strlen(command) - 1] = '\0'; // remove the \n
    int len_1;
    for(len_1 = 0; command[len_1] != '\0'; len_1++)
    {
      if(command[len_1] == ' ')
      {
        break;
      }
    }
    parsed_command[0] = command;
    if(len_1 == strlen(command))
    {
      printf("Command is '%s' with no arguments\n", parsed_command[0]);
      parsed_command[1] = NULL;

      /* Handle built-in exit command */
      if(!strcmp(parsed_command[0], "quit") || !strcmp(parsed_command[0], "exit"))
      {
        exit(EXIT_SUCCESS);
      }
    }
    else
    {
      command[len_1] = '\0';
      parsed_command[1] = command + len_1 + 1;
      printf("Command is '%s' with argument '%s'\n", parsed_command[0], parsed_command[1]);

      /* Handle built-in exit command */
      if(!strcmp(parsed_command[0], "quit") || !strcmp(parsed_command[0], "exit"))
      {
        exit(EXIT_SUCCESS);
      }
    }

    int status = -1;
    pid_t pid;
    int is_bg = strncmp(parsed_command[0], "BG", 2);

    pid = fork();
    if(pid == 0)
    {
      char *cmd = parsed_command[0];

      // Child
      if(is_bg == 0)
      {
        cmd = parsed_command[0] + 2;
        // AI Review Feedback: The intermediate watcher process inherits handle_sigchld from the
        // parent. When the grandchild exits, SIGCHLD triggers handle_sigchld in the watcher,
        // potentially competing with the watcher's own wait(&status). Resetting signal(SIGCHLD,
        // SIG_DFL) in the watcher avoids this unexpected signal handler interference.
        pid_t bg_pid = fork();
        if(bg_pid > 0)
        {
          wait(&status);
          printf("Background command finished %d\n", WEXITSTATUS(status));
          exit(EXIT_SUCCESS);
        }
      }

      execlp(cmd, cmd, parsed_command[1], NULL);
      perror("execlp");
      // AI Review Feedback: Exiting with EXIT_SUCCESS when execlp fails incorrectly signals clean
      // execution to the parent/watcher process; should exit with EXIT_FAILURE or 127.
      exit(EXIT_SUCCESS);
    }
    else
    {
      // Parent
      if(is_bg == 0)
      {
        printf("BG command, no waiting on child\n");
      }
      else
      {
        wait(&status);
        /* Extract and display the child process exit status code */
        printf("The child is back and it returned %d\n", WEXITSTATUS(status));
      }
    }
  }
}
