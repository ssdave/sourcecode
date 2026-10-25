/**
 * @file i2c_cdev.h
 * @brief Header definitions for UNIX Virtual File System (VFS) Character Device Subsystem for I2C.
 *
 * Architectural Concepts:
 * 1. Virtual File System (VFS) character device abstraction (struct inode, struct file, and struct
 * file_operations).
 * 2. User-Kernel address space isolation and boundary validation (copy_to_user, copy_from_user,
 * -EFAULT).
 * 3. File descriptor table management and per-open file session state (filp->private_data).
 * 4. Driver ioctl dispatch (I2C_SLAVE address configuration, I2C_GET_STATS, I2C_RESET_BUS).
 * 5. Concurrent hardware bus serialization using POSIX mutex locking across multiple calling
 * threads.
 * 6. Integration with mock I2C peripheral hardware (LM75 temperature sensor register file).
 *
 * Academic Attribution:
 * Exercise design based on UNIX device driver architectures and the Linux kernel VFS subsystem
 * (OSTEP Chapters 36 & 39, CS:APP Chapter 10, Corbet/Rubini/Kroah-Hartman "Linux Device Drivers",
 * and Linux kernel drivers/i2c/i2c-dev.c).
 * Retained strictly for independent educational self-study.
 */

#ifndef I2C_CDEV_H
#define I2C_CDEV_H

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* ==============================================================================
 * Protocol Definitions & System Constants
 * ============================================================================== */

#define I2C_MAJOR 89         /* Linux standard major number for I2C character devices */
#define I2C_MIN_MINOR 0      /* Default minor number for /dev/i2c-0 */
#define MAX_FDS 64           /* Maximum supported open file descriptors */
#define MAX_PATH_LEN 64      /* Maximum path length for simulated devfs */
#define MAX_TRANSFER_LEN 256 /* Maximum byte payload per I2C transaction */

/* Standard I2C Character Device ioctl Commands */
#define I2C_SLAVE 0x0703     /* Set target 7-bit peripheral slave address */
#define I2C_GET_STATS 0x0780 /* Retrieve bus transfer statistics */
#define I2C_RESET_BUS 0x0781 /* Trigger hardware bus reset and line release */

/* User memory fault injection poison address */
#define FAULT_INJECTION_ADDR ((void *)0xDEADBEEF)

typedef int64_t loff_t;

/* Forward declarations */
struct cdev;
struct inode;
struct file;

/**
 * UNIX Driver Switch Table (struct file_operations).
 */
struct file_operations
{
  int (*open)(struct inode *inode, struct file *filp);
  int (*release)(struct inode *inode, struct file *filp);
  ssize_t (*read)(struct file *filp, char *buf, size_t count, loff_t *f_pos);
  ssize_t (*write)(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
  long (*unlocked_ioctl)(struct file *filp, unsigned int cmd, unsigned long arg);
};

/**
 * UNIX Character Device (struct cdev).
 */
struct cdev
{
  dev_t devno;
  const struct file_operations *ops;
  void *driver_data;
};

/**
 * UNIX Virtual File System Inode (struct inode).
 */
struct inode
{
  dev_t i_rdev;
  unsigned int i_major;
  unsigned int i_minor;
  struct cdev *i_cdev;
};

/**
 * UNIX Open File Context (struct file).
 */
struct file
{
  loff_t f_pos;
  unsigned int f_flags;
  void *private_data;
  const struct file_operations *f_op;
  struct inode *f_inode;
};

/**
 * I2C Bus Cumulative Transfer Statistics.
 */
typedef struct
{
  uint32_t total_transfers;
  uint32_t bytes_written;
  uint32_t bytes_read;
  uint32_t nack_count;
} i2c_bus_stats_t;

/**
 * Per-Open Session Context (attached to filp->private_data).
 */
typedef struct
{
  uint8_t slave_addr;      /* Configured 7-bit target slave address */
  bool slave_set;          /* True once I2C_SLAVE ioctl has been called */
  uint32_t session_reads;  /* Total bytes read in this open session */
  uint32_t session_writes; /* Total bytes written in this open session */
} i2c_cdev_session_t;

/**
 * Mock I2C Slave Peripheral (LM75 / TMP102 Temperature Sensor Compatible).
 */
typedef struct
{
  uint8_t address;      /* 7-bit slave address (e.g., 0x48) */
  uint8_t selected_reg; /* Current register pointer offset */
  uint8_t registers[8]; /* Internal register file */
} mock_i2c_slave_t;

/**
 * Physical I2C Bus Controller & Device Model.
 */
typedef struct
{
  pthread_mutex_t bus_lock; /* Mutex protecting concurrent physical bus transactions */
  mock_i2c_slave_t slave;   /* Attached mock peripheral */
  i2c_bus_stats_t stats;    /* Transfer statistics counters */
  dev_t devno;              /* Major/minor device identifier */
  char name[32];            /* Controller device name */
} i2c_cdev_device_t;

/**
 * Simulated DevFS Device Node Entry.
 */
typedef struct
{
  char path[MAX_PATH_LEN];
  struct inode inode;
  struct cdev cdev;
  bool registered;
} devfs_entry_t;

/* ==============================================================================
 * Global Subsystem State Declarations
 * ============================================================================== */

extern devfs_entry_t g_devfs_table[16];
extern size_t g_devfs_count;
extern struct file *g_fd_table[MAX_FDS];
extern pthread_mutex_t g_fd_mutex;
extern i2c_cdev_device_t g_i2c_controller;
extern const struct file_operations g_i2c_fops;

/* ==============================================================================
 * Function Prototypes
 * ============================================================================== */

/* Mock Hardware Functions */
void mock_slave_init(mock_i2c_slave_t *slv, uint8_t address);
int i2c_hardware_transfer(i2c_cdev_device_t *dev, uint8_t addr, uint8_t *buf, size_t count,
                          bool is_read);
void i2c_bus_reset(i2c_cdev_device_t *dev);

/* User-Kernel Isolation & Memory Copy Primitives */
bool is_valid_user_ptr(const void *ptr, size_t n);
int copy_to_user(void *user_dest, const void *kernel_src, size_t n);
int copy_from_user(void *kernel_dest, const void *user_src, size_t n);

/* VFS DevFS & Lifecycle Management */
int vfs_register_cdev(const char *path, dev_t devno, const struct file_operations *fops,
                      void *driver_data);
struct inode *vfs_lookup(const char *path);
void i2c_subsystem_init(void);
void i2c_subsystem_cleanup(void);

/* VFS Simulated System Calls */
int sys_cdev_open(const char *pathname, int flags);
int sys_cdev_close(int fd);
ssize_t sys_cdev_read(int fd, void *buf, size_t count);
ssize_t sys_cdev_write(int fd, const void *buf, size_t count);
int sys_cdev_ioctl(int fd, unsigned long cmd, unsigned long arg);

/* I2C Driver Switch Table Callbacks (file_operations) */
int i2c_cdev_open(struct inode *inode, struct file *filp);
int i2c_cdev_release(struct inode *inode, struct file *filp);
long i2c_cdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
ssize_t i2c_cdev_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t i2c_cdev_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

#endif /* I2C_CDEV_H */
