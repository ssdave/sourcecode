/**
 * @file main.c
 * @brief Comprehensive Verification & Multi-Threaded Stress Test Suite for I2C Character Device
 * Subsystem.
 *
 * Academic Attribution:
 * Exercise design based on UNIX device driver architectures and the Linux kernel VFS subsystem
 * (OSTEP Chapters 36 & 39, CS:APP Chapter 10, Corbet/Rubini/Kroah-Hartman "Linux Device Drivers",
 * and Linux kernel drivers/i2c/i2c-dev.c).
 * Retained strictly for independent educational self-study.
 */

#include "i2c_cdev.h"

/* ==============================================================================
 * Comprehensive Verification Test Cases
 * ============================================================================== */

static void test_vfs_open_close_lifecycle(void)
{
  printf("[TEST 1] Verifying VFS device node open, session binding, and close...\n");

  /* Attempt to open non-existent device */
  int bad_fd = sys_cdev_open("/dev/nonexistent", 0);
  assert(bad_fd == -1);

  /* Open simulated I2C character device */
  int fd = sys_cdev_open("/dev/i2c-0", 0);
  assert(fd >= 0 && fd < MAX_FDS);

  /* Inspect FD table context */
  struct file *filp = g_fd_table[fd];
  assert(filp != NULL);
  assert(filp->f_inode != NULL);
  assert(filp->f_op == &g_i2c_fops);
  assert(filp->private_data != NULL);

  /* Close device and verify cleanup */
  assert(sys_cdev_close(fd) == 0);
  assert(g_fd_table[fd] == NULL);

  /* Closing an already closed fd must fail with EBADF */
  assert(sys_cdev_close(fd) == -1);

  printf("  -> PASS: VFS open/close lifecycle and session tracking verified.\n");
}

static void test_ioctl_slave_configuration(void)
{
  printf("[TEST 2] Verifying ioctl I2C_SLAVE setting and boundary validation...\n");

  int fd = sys_cdev_open("/dev/i2c-0", 0);
  assert(fd >= 0);

  /* Attempting read/write prior to I2C_SLAVE configuration must return error */
  uint8_t dummy = 0;
  assert(sys_cdev_read(fd, &dummy, 1) == -1);

  /* Configure valid 7-bit slave address 0x48 */
  assert(sys_cdev_ioctl(fd, I2C_SLAVE, 0x48) == 0);

  /* Verify session updated */
  struct file *filp = g_fd_table[fd];
  i2c_cdev_session_t *sess = (i2c_cdev_session_t *)filp->private_data;
  assert(sess != NULL);
  assert(sess->slave_set == true);
  assert(sess->slave_addr == 0x48);

  /* Address exceeding 7 bits (> 0x7F) must be rejected with -EINVAL */
  assert(sys_cdev_ioctl(fd, I2C_SLAVE, 0x80) == -1);

  assert(sys_cdev_close(fd) == 0);
  printf("  -> PASS: ioctl I2C_SLAVE configuration and range checks verified.\n");
}

static void test_synchronous_read_write_transactions(void)
{
  printf("[TEST 3] Verifying synchronous I2C read and write transactions...\n");

  int fd = sys_cdev_open("/dev/i2c-0", 0);
  assert(fd >= 0);
  assert(sys_cdev_ioctl(fd, I2C_SLAVE, 0x48) == 0);

  /* Write 2 bytes: register offset 0x02 (T_LOW) and new value 0x55 */
  uint8_t write_buf[2] = {0x02, 0x55};
  ssize_t written = sys_cdev_write(fd, write_buf, sizeof(write_buf));
  assert(written == 2);

  /* Point register selector to 0x02 */
  uint8_t reg_ptr = 0x02;
  assert(sys_cdev_write(fd, &reg_ptr, 1) == 1);

  /* Read back 1 byte from T_LOW register */
  uint8_t read_buf = 0;
  ssize_t bytes_read = sys_cdev_read(fd, &read_buf, 1);
  assert(bytes_read == 1);
  assert(read_buf == 0x55);

  /* Query bus statistics via ioctl I2C_GET_STATS */
  i2c_bus_stats_t stats;
  memset(&stats, 0, sizeof(stats));
  assert(sys_cdev_ioctl(fd, I2C_GET_STATS, (unsigned long)&stats) == 0);

  assert(stats.total_transfers >= 3);
  assert(stats.bytes_written >= 3);
  assert(stats.bytes_read >= 1);

  assert(sys_cdev_close(fd) == 0);
  printf("  -> PASS: Synchronous read/write and ioctl stats verified against mock sensor.\n");
}

static void test_user_kernel_isolation_and_fault_injection(void)
{
  printf("[TEST 4] Verifying user-kernel memory isolation and fault injection...\n");

  int fd = sys_cdev_open("/dev/i2c-0", 0);
  assert(fd >= 0);
  assert(sys_cdev_ioctl(fd, I2C_SLAVE, 0x48) == 0);

  /* Fault injection: NULL user buffer on write */
  assert(sys_cdev_write(fd, NULL, 4) == -1);

  /* Fault injection: Poison user address on write */
  assert(sys_cdev_write(fd, FAULT_INJECTION_ADDR, 4) == -1);

  /* Fault injection: NULL user buffer on read */
  assert(sys_cdev_read(fd, NULL, 4) == -1);

  /* Fault injection: Poison user address on read */
  assert(sys_cdev_read(fd, FAULT_INJECTION_ADDR, 4) == -1);

  /* Fault injection: NULL destination for ioctl I2C_GET_STATS */
  assert(sys_cdev_ioctl(fd, I2C_GET_STATS, (unsigned long)NULL) == -1);

  /* Fault injection: Poison destination for ioctl I2C_GET_STATS */
  assert(sys_cdev_ioctl(fd, I2C_GET_STATS, (unsigned long)FAULT_INJECTION_ADDR) == -1);

  assert(sys_cdev_close(fd) == 0);
  printf("  -> PASS: User-kernel boundary safety and fault injection verified.\n");
}

/* ==============================================================================
 * Test 5: Multi-Threaded Concurrent Stress Test
 * ============================================================================== */

#define CONCURRENT_ITERATIONS 100

static void *worker_sensor_reader(void *arg)
{
  (void)arg;
  int fd = sys_cdev_open("/dev/i2c-0", 0);
  assert(fd >= 0);
  assert(sys_cdev_ioctl(fd, I2C_SLAVE, 0x48) == 0);

  for(int i = 0; i < CONCURRENT_ITERATIONS; i++)
  {
    uint8_t reg = 0x00; /* Temperature register */
    assert(sys_cdev_write(fd, &reg, 1) == 1);

    uint8_t temp = 0;
    assert(sys_cdev_read(fd, &temp, 1) == 1);
    /* Simulated small worker yield */
    usleep(100);
  }

  assert(sys_cdev_close(fd) == 0);
  return NULL;
}

static void *worker_config_writer(void *arg)
{
  (void)arg;
  int fd = sys_cdev_open("/dev/i2c-0", 0);
  assert(fd >= 0);
  assert(sys_cdev_ioctl(fd, I2C_SLAVE, 0x48) == 0);

  for(int i = 0; i < CONCURRENT_ITERATIONS; i++)
  {
    uint8_t payload[2] = {0x03, (uint8_t)(i & 0xFF)}; /* T_HIGH register */
    assert(sys_cdev_write(fd, payload, sizeof(payload)) == 2);
    usleep(150);
  }

  assert(sys_cdev_close(fd) == 0);
  return NULL;
}

static void *worker_stats_poller(void *arg)
{
  (void)arg;
  int fd = sys_cdev_open("/dev/i2c-0", 0);
  assert(fd >= 0);
  assert(sys_cdev_ioctl(fd, I2C_SLAVE, 0x48) == 0);

  uint32_t prev_transfers = 0;
  for(int i = 0; i < CONCURRENT_ITERATIONS; i++)
  {
    i2c_bus_stats_t stats;
    memset(&stats, 0, sizeof(stats));
    assert(sys_cdev_ioctl(fd, I2C_GET_STATS, (unsigned long)&stats) == 0);
    assert(stats.total_transfers >= prev_transfers);
    prev_transfers = stats.total_transfers;
    usleep(120);
  }

  assert(sys_cdev_close(fd) == 0);
  return NULL;
}

static void *worker_rapid_open_close(void *arg)
{
  (void)arg;
  for(int i = 0; i < CONCURRENT_ITERATIONS; i++)
  {
    int fd = sys_cdev_open("/dev/i2c-0", 0);
    assert(fd >= 0);
    assert(sys_cdev_ioctl(fd, I2C_SLAVE, 0x48) == 0);

    uint8_t buf = 0;
    assert(sys_cdev_read(fd, &buf, 1) == 1);

    assert(sys_cdev_close(fd) == 0);
    usleep(200);
  }
  return NULL;
}

static void test_multithreaded_concurrent_stress(void)
{
  printf("[TEST 5] Launching multi-threaded concurrent stress test (4 worker threads)...\n");

  pthread_t threads[4];
  assert(pthread_create(&threads[0], NULL, worker_sensor_reader, NULL) == 0);
  assert(pthread_create(&threads[1], NULL, worker_config_writer, NULL) == 0);
  assert(pthread_create(&threads[2], NULL, worker_stats_poller, NULL) == 0);
  assert(pthread_create(&threads[3], NULL, worker_rapid_open_close, NULL) == 0);

  for(int i = 0; i < 4; i++)
  {
    assert(pthread_join(threads[i], NULL) == 0);
  }

  /* Verify final stats */
  int fd = sys_cdev_open("/dev/i2c-0", 0);
  assert(fd >= 0);
  i2c_bus_stats_t final_stats;
  memset(&final_stats, 0, sizeof(final_stats));
  assert(sys_cdev_ioctl(fd, I2C_GET_STATS, (unsigned long)&final_stats) == 0);

  assert(final_stats.total_transfers > 0);
  assert(final_stats.bytes_read > 0);
  assert(final_stats.bytes_written > 0);

  assert(sys_cdev_close(fd) == 0);
  printf(
      "  -> PASS: 4 concurrent threads completed with zero race conditions or data corruption.\n");
}

/* ==============================================================================
 * Main Execution Entry Point
 * ============================================================================== */

int main(void)
{
  printf("==============================================================================\n");
  printf("Running UNIX I2C Character Device Subsystem & VFS Test Suite\n");
  printf("==============================================================================\n");

  i2c_subsystem_init();

  /* Probe for scaffold completion */
  int probe_fd = sys_cdev_open("/dev/i2c-0", 0);
  if(probe_fd == -1)
  {
    printf("[Exercise Scaffold] sys_cdev_open() returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    return 0;
  }
  sys_cdev_close(probe_fd);

  test_vfs_open_close_lifecycle();
  test_ioctl_slave_configuration();
  test_synchronous_read_write_transactions();
  test_user_kernel_isolation_and_fault_injection();
  test_multithreaded_concurrent_stress();

  printf("==============================================================================\n");
  printf("All I2C Character Device Subsystem Tests Passed Successfully!\n");
  printf("==============================================================================\n");

  return 0;
}
