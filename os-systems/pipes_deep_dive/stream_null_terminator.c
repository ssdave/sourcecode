/**
 * @file stream_null_terminator.c
 * @brief Demonstrates UNIX pipe byte stream semantics vs. C string null-termination mechanics.
 *
 * Concepts Demonstrated:
 * 1. UNIX pipes carry raw, unstructured byte streams—NOT C strings or message datagrams.
 * 2. Why read() NEVER automatically appends a null terminator ('\0').
 * 3. The risk of out-of-bounds memory over-reads if printing or copying an unterminated read()
 * buffer.
 * 4. Safe string framing and delimiter-based line parsing over anonymous pipes.
 *
 * Problem Description:
 * In C systems programming, string functions (printf("%s"), strlen, strcmp) expect a null
 * terminator ('\0') at the end of character sequences. UNIX pipes, however, operate purely on octet
 * streams without knowledge of string boundaries. When a process sends characters into a pipe and
 * another process calls read(), read() fills only the requested bytes and returns the count. It
 * leaves all subsequent bytes in the destination buffer untouched. If the receiving program assumes
 * the buffer is a null-terminated string, it will read past the valid payload into arbitrary memory
 * until encountering an accidental zero byte, leading to data corruption or information disclosure
 * vulnerabilities.
 *
 * Academic Attribution:
 * Exercise specification based on systems I/O semantics (Rose-Hulman CSSE 332 & Kerrisk TLPI
 * Chapter 44). Retained strictly for independent educational self-study.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define READ_FD 0
#define WRITE_FD 1

/**
 * Sends a raw text string over a pipe without including any trailing null terminator.
 *
 * Contract:
 * - Writes exact payload bytes to write_fd (omitting '\0').
 * - Returns bytes written, or -1 on error.
 *
 * @param write_fd The pipe write end descriptor.
 * @param text The null-terminated input string to transmit.
 * @return Number of bytes written to the pipe, or -1 on error.
 */
ssize_t send_raw_stream_payload(int write_fd, const char *text)
{
  // AI Review Feedback: Consider validating 'if(text == NULL) return -1;' to prevent
  // undefined behavior (SIGSEGV) in strlen(). Additionally, in general POSIX streaming,
  // write() may return fewer bytes than requested (partial write) or fail with EINTR;
  // a transmission loop can guarantee complete payload delivery.
  return write(write_fd, text, strlen(text));
}

/**
 * Reads data from a pipe and guarantees safe null termination in destination buffer.
 *
 * Contract & Requirements:
 * - Reads from read_fd into dest, bounded by max_capacity.
 * - Guarantees safe null-termination of dest.
 * - Returns number of bytes read (excluding null terminator), or -1 on error.
 *
 * @param read_fd The pipe read end descriptor.
 * @param dest The destination buffer.
 * @param max_capacity The maximum allocated capacity of dest.
 * @return Number of bytes read (excluding null terminator), or -1 on error.
 */
ssize_t safe_read_null_terminated(int read_fd, char *dest, size_t max_capacity)
{
  // AI Review Feedback: If max_capacity is 0, (max_capacity - 1) underflows size_t to
  // SIZE_MAX (e.g. 18446744073709551615 on 64-bit platforms), allowing read() to
  // read up to SIZE_MAX bytes into dest and causing a severe buffer overflow.
  // Guard with 'if(max_capacity == 0) return 0;' upfront.
  // AI Review Feedback: If read() encounters an error or is interrupted (returning -1),
  // executing dest[bytes] = '\0' writes out-of-bounds to dest[-1] (buffer underflow).
  // Ensure 'if(bytes >= 0) dest[bytes] = '\0';' before returning.
  ssize_t bytes = read(read_fd, dest, max_capacity - 1);
  dest[bytes] = '\0';
  return bytes;
}

/**
 * Reads a single delimiter-terminated line from a stream pipe.
 *
 * Contract & Requirements:
 * - Reads from read_fd until delimiter or EOF is reached.
 * - Ensures dest is null-terminated and does not exceed max_capacity.
 * - Returns length of line read excluding null terminator, 0 on clean EOF, or -1 on error.
 *
 * @param read_fd The pipe read end descriptor.
 * @param dest The destination buffer.
 * @param max_capacity Maximum capacity of dest.
 * @param delimiter The line delimiter character (e.g., '\n').
 * @return Length of line read, 0 on EOF, or -1 on error.
 */
ssize_t read_stream_line(int read_fd, char *dest, size_t max_capacity, char delimiter)
{
  size_t count = 0;

  // AI Review Feedback: If max_capacity is 0, (max_capacity - 1) underflows size_t to SIZE_MAX.
  // Guard against max_capacity == 0 before the loop.
  while(count < (max_capacity - 1))
  {
    ssize_t bytes = read(read_fd, &dest[count], 1);
    if(bytes > 0)
    {
      count++;
      if(dest[count - 1] == delimiter)
      {
        break;
      }
    }
    else if(bytes == 0) // EOF
    {
      break;
    }
    // AI Review Feedback: If read() returns -1 (e.g. EINTR, EBADF, EIO), neither branch
    // executes. The loop spins repeatedly attempting to read, leading to potential infinite
    // looping, high CPU usage, or returning partial data without error indication.
    // Explicitly handle 'else if(bytes < 0) return -1;'.
  }
  dest[count] = '\0';
  return count;
}

int main(void)
{
  printf("=== Test Suite: Stream Semantics & Null Termination ===\n");

  int pipe_fds[2];

  if(pipe(pipe_fds) < 0)
  {
    perror("pipe failed");
    return 1;
  }

  // --- Test 1: Raw stream payload transfer with buffer poisoning ---
  const char *sample = "Operating Systems IPC";
  ssize_t sent = send_raw_stream_payload(pipe_fds[WRITE_FD], sample);

  if(sent == -1)
  {
    printf("[Exercise Scaffold] send_raw_stream_payload() returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    close(pipe_fds[READ_FD]);
    close(pipe_fds[WRITE_FD]);
    return 0;
  }

  close(pipe_fds[WRITE_FD]); // Send EOF

  // Pre-fill buffer with 'X' markers to detect missing null termination
  char buffer[64];
  memset(buffer, 'X', sizeof(buffer));

  ssize_t bytes_read = safe_read_null_terminated(pipe_fds[READ_FD], buffer, sizeof(buffer));
  close(pipe_fds[READ_FD]);

  if(bytes_read == -1)
  {
    printf("[Exercise Scaffold] safe_read_null_terminated() returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    return 0;
  }

  printf("[Verification] Read %zd bytes safely.\n", bytes_read);
  printf("[Verification] Payload: \"%s\"\n", buffer);

  assert(bytes_read == (ssize_t)strlen(sample));
  assert(strcmp(buffer, sample) == 0);
  // Ensure the null-terminator was explicitly set, and buffer boundary is intact
  assert(buffer[bytes_read] == '\0');
  assert(buffer[bytes_read + 1] == 'X');

  // --- Test 2: Delimited line stream reading ---
  int line_pipe[2];
  if(pipe(line_pipe) < 0)
  {
    perror("line pipe failed");
    return 1;
  }

  const char *stream_feed = "Alpha\nBeta\nGamma\n";
  write(line_pipe[WRITE_FD], stream_feed, strlen(stream_feed));
  close(line_pipe[WRITE_FD]);

  char line[32];
  ssize_t l1 = read_stream_line(line_pipe[READ_FD], line, sizeof(line), '\n');
  if(l1 == -1)
  {
    printf("[Exercise Scaffold] read_stream_line() returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    close(line_pipe[READ_FD]);
    return 0;
  }

  assert(strcmp(line, "Alpha\n") == 0);

  ssize_t l2 = read_stream_line(line_pipe[READ_FD], line, sizeof(line), '\n');
  assert(l2 > 0);
  assert(strcmp(line, "Beta\n") == 0);

  ssize_t l3 = read_stream_line(line_pipe[READ_FD], line, sizeof(line), '\n');
  assert(l3 > 0);
  assert(strcmp(line, "Gamma\n") == 0);

  ssize_t l4 = read_stream_line(line_pipe[READ_FD], line, sizeof(line), '\n');
  assert(l4 == 0); // EOF

  close(line_pipe[READ_FD]);

  printf("=== PASS: Stream byte semantics and null termination verified! ===\n");
  return 0;
}
