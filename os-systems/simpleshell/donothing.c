/* ============================================================================
 * File: donothing.c
 *
 * Attribution & Copyright:
 *   Originates from Rose-Hulman Institute of Technology — CSSE 332: Operating Systems
 *   Based on CAOS Lab from Jacobs University Bremen.
 *   Course Schedule: https://www.rose-hulman.edu/class/csse/csse332/2324b/schedule/
 *   Lab Archive:     https://github.com/rhit-csse332/csse332-labs-archive
 *
 * What is being built / tested:
 *   Helper child binary invoked by `simpleshell` that prints its executable name
 *   and command-line arguments, sleeps 5 seconds, and exits—used to test
 *   foreground `wait()` blocking and background (`BG`) asynchronous execution.
 * ============================================================================ */

#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{

  printf("Do nothing program started!\n");
  printf("EXECUTABLE '%s'\n", argv[0]);
  for(int i = 1; i < argc; i++)
  {
    printf("ARGUMENT %d: %s\n", i, argv[i]);
  }
  printf("Pausing for 5 seconds...\n");
  sleep(5);
  printf("Do nothing program finished!\n");
  return 0;
}
