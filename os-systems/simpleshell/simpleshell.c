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
 *   and `BG`-prefixed background commands.
 * ============================================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
  char command[82];
  char *parsed_command[2];
  // takes at most two input arguments
  // infinite loop but ^C quits
  while(1)
  {
    printf("SHELL%% ");
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

      // leave this here, do not change it
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

      // leave this here, do not change it
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
        /* Use the WEXITSTATUS macro to read the child's return code. */
        printf("The child is back and it returned %d\n", WEXITSTATUS(status));
      }
    }
  }
}
