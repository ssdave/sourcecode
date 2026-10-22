/**
 * @file prime_sieve_pipeline.c
 * @brief Dynamic Multi-Process Prime Sieve Pipeline (Sieve of Eratosthenes via UNIX Pipes).
 *
 * An on-demand pipeline architecture originally conceived by Doug McIlroy (inventor of UNIX pipes).
 * Each discovered prime number spawns a new dedicated filter process in the pipeline.
 *
 * Architecture:
 *   [Main / Generator (2..35)]
 *              |
 *            (pipe)
 *              v
 *      [Filter: Prime 2] ---> drops multiples of 2
 *              |
 *            (pipe)
 *              v
 *      [Filter: Prime 3] ---> drops multiples of 3
 *              |
 *            (pipe)
 *              v
 *      [Filter: Prime 5] ---> ...
 *
 * Requirements:
 * 1. Print all primes <= 35 (i.e., 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31).
 * 2. Processes must be spawned strictly on a "per-need" basis (not pre-allocated).
 * 3. Each prime must be printed by its own dedicated process.
 * 4. The entire pipeline must terminate cleanly with zero zombie or orphan processes.
 *
 * Academic Attribution:
 * Concept: Doug McIlroy (Bell Labs / Dartmouth).
 * Exercise specification: Rose-Hulman CSSE 332 (Operating Systems) & MIT 6.S081.
 * Retained strictly for independent educational self-study.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ_FD 0
#define WRITE_FD 1
#define MAX_NUM 35

/**
 * Worker process representing a stage in the dynamic prime filter pipeline.
 *
 * Contract:
 * - Reads integers from left_read_fd.
 * - Identifies prime numbers and filters out their multiples.
 * - Creates downstream stages dynamically as new primes are discovered.
 * - Ensures clean pipeline shutdown, descriptor closure, and child reaping.
 *
 * @param left_read_fd Pipe read descriptor from the upstream neighbor.
 */
void sieve_worker(int left_read_fd)
{
  (void)left_read_fd;
  /* TODO: Author Implementation */
}

int main(void)
{
  int initial_pipe[2];

  if(pipe(initial_pipe) < 0)
  {
    perror("initial pipe failed");
    return 1;
  }

  pid_t first_worker = fork();

  if(first_worker < 0)
  {
    perror("initial fork failed");
    return 1;
  }

  if(first_worker == 0)
  {
    // First worker: reads from initial_pipe[READ_FD]
    close(initial_pipe[WRITE_FD]);
    sieve_worker(initial_pipe[READ_FD]);
    exit(0);
  }

  // Generator Process: close read end
  close(initial_pipe[READ_FD]);

  // Feed numbers 2 through MAX_NUM into the pipeline
  for(int i = 2; i <= MAX_NUM; i++)
  {
    write(initial_pipe[WRITE_FD], &i, sizeof(i));
  }

  // Close write end to signal EOF through the pipeline
  close(initial_pipe[WRITE_FD]);

  // Wait for the entire pipeline to terminate cleanly
  waitpid(first_worker, NULL, 0);

  return 0;
}
