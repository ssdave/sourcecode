/**
 * @file cdev_vfs.c
 * @brief Virtual File System (VFS) switch layer and user-kernel memory isolation primitives.
 *
 * Academic Attribution:
 * Exercise design based on UNIX device driver architectures and the Linux kernel VFS subsystem
 * (OSTEP Chapters 36 & 39, CS:APP Chapter 10, Corbet/Rubini/Kroah-Hartman "Linux Device Drivers",
 * and Linux kernel drivers/i2c/i2c-dev.c).
 * Retained strictly for independent educational self-study.
 */

#include "i2c_cdev.h"

devfs_entry_t g_devfs_table[16];
size_t g_devfs_count = 0;

struct file *g_fd_table[MAX_FDS];
pthread_mutex_t g_fd_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Registers a character device in the simulated devfs node registry.
 *
 * @param path Device node path (e.g., "/dev/i2c-0").
 * @param devno Major and minor device number pair.
 * @param fops Driver switch table operations pointer.
 * @param driver_data Context pointer to bind to cdev.
 * @return 0 on success, or -EINVAL on invalid parameters or table full.
 */
int vfs_register_cdev(const char *path, dev_t devno, const struct file_operations *fops,
                      void *driver_data)
{
  if(path == NULL || fops == NULL || g_devfs_count >= 16)
  {
    return -EINVAL;
  }

  devfs_entry_t *entry = &g_devfs_table[g_devfs_count++];
  strncpy(entry->path, path, sizeof(entry->path) - 1);
  entry->path[sizeof(entry->path) - 1] = '\0';

  entry->cdev.devno = devno;
  entry->cdev.ops = fops;
  entry->cdev.driver_data = driver_data;

  entry->inode.i_rdev = devno;
  entry->inode.i_major = (devno >> 8) & 0xFF;
  entry->inode.i_minor = devno & 0xFF;
  entry->inode.i_cdev = &entry->cdev;
  entry->registered = true;

  return 0;
}

/**
 * Looks up a simulated devfs entry by path.
 *
 * @param path Target device node path.
 * @return Pointer to struct inode on success, or NULL if not found.
 */
struct inode *vfs_lookup(const char *path)
{
  if(path == NULL)
  {
    return NULL;
  }

  for(size_t i = 0; i < g_devfs_count; i++)
  {
    if(g_devfs_table[i].registered && strcmp(g_devfs_table[i].path, path) == 0)
    {
      return &g_devfs_table[i].inode;
    }
  }

  return NULL;
}

/**
 * Validates a simulated user memory pointer for isolation checks.
 *
 * @param ptr Address to validate.
 * @param n Length of memory buffer in bytes.
 * @return true if valid; false if invalid (NULL, poison address, or overflow).
 */
bool is_valid_user_ptr(const void *ptr, size_t n)
{
  if(ptr == NULL)
  {
    return false;
  }
  if(ptr == FAULT_INJECTION_ADDR)
  {
    return false;
  }
  if((uintptr_t)ptr < 0x1000)
  {
    return false;
  }
  if((uintptr_t)ptr + n < (uintptr_t)ptr)
  {
    return false;
  }
  return true;
}

/**
 * Bootstraps the I2C character device subsystem and mock hardware.
 */
void i2c_subsystem_init(void)
{
  memset(&g_i2c_controller, 0, sizeof(g_i2c_controller));
  pthread_mutex_init(&g_i2c_controller.bus_lock, NULL);
  mock_slave_init(&g_i2c_controller.slave, 0x48); /* LM75 at address 0x48 */

  dev_t devno = (I2C_MAJOR << 8) | I2C_MIN_MINOR;
  g_i2c_controller.devno = devno;
  strncpy(g_i2c_controller.name, "i2c-0", sizeof(g_i2c_controller.name) - 1);

  vfs_register_cdev("/dev/i2c-0", devno, &g_i2c_fops, &g_i2c_controller);
}

/**
 * Tears down the I2C character device subsystem and reclaims allocated contexts.
 */
void i2c_subsystem_cleanup(void)
{
  pthread_mutex_destroy(&g_i2c_controller.bus_lock);

  pthread_mutex_lock(&g_fd_mutex);
  for(int i = 0; i < MAX_FDS; i++)
  {
    if(g_fd_table[i] != NULL)
    {
      if(g_fd_table[i]->private_data != NULL)
      {
        free(g_fd_table[i]->private_data);
      }
      free(g_fd_table[i]);
      g_fd_table[i] = NULL;
    }
  }
  pthread_mutex_unlock(&g_fd_mutex);
  g_devfs_count = 0;
}

/* ==============================================================================
 * Exercise Skeletons: User-Kernel Isolation & Memory Copy Primitives
 * ============================================================================== */

/**
 * Copies a memory buffer from kernel address space to user address space.
 *
 * Architectural Invariant:
 * Enforces hardware memory isolation. Validates destination user address bounds
 * before initiating transfer. Must never trigger a kernel segmentation violation
 * if the user provides an invalid, unmapped, or poison pointer.
 *
 * @param user_dest Destination pointer in user address space.
 * @param kernel_src Source pointer in kernel address space.
 * @param n Number of bytes to copy.
 * @return 0 on success, or -EFAULT if user_dest is invalid or NULL.
 */
int copy_to_user(void *user_dest, const void *kernel_src, size_t n)
{
  if((user_dest == NULL) || (kernel_src == NULL) || (n == 0))
  {
    return -EFAULT;
  }

  if(!is_valid_user_ptr(user_dest, n))
  {
    return -EFAULT;
  }

  memcpy(user_dest, kernel_src, n);
  return 0;
}

/**
 * Copies a memory buffer from user address space to kernel address space.
 *
 * Architectural Invariant:
 * Enforces hardware memory isolation. Validates source user address bounds
 * before initiating transfer. Rejects NULL, poison, or invalid addresses.
 *
 * @param kernel_dest Destination pointer in kernel address space.
 * @param user_src Source pointer in user address space.
 * @param n Number of bytes to copy.
 * @return 0 on success, or -EFAULT if user_src is invalid or NULL.
 */
int copy_from_user(void *kernel_dest, const void *user_src, size_t n)
{
  if((kernel_dest == NULL) || (user_src == NULL) || (n == 0))
  {
    return -EFAULT;
  }

  if(!is_valid_user_ptr(user_src, n))
  {
    return -EFAULT;
  }

  memcpy(kernel_dest, user_src, n);
  return 0;
}

/* ==============================================================================
 * Exercise Skeletons: VFS Simulated System Calls
 * ============================================================================== */

/**
 * Simulated VFS open() system call.
 *
 * Interface Contract:
 * Resolves pathname in simulated DevFS, allocates a file object, assigns an unused
 * file descriptor in the table under lock, and invokes the driver open operation.
 *
 * @param pathname Path to device node (e.g. "/dev/i2c-0").
 * @param flags Open mode flags.
 * @return Non-negative integer file descriptor (fd >= 0) on success, or -1 on error (setting
 * errno).
 */
int sys_cdev_open(const char *pathname, int flags)
{
  /* TODO: Author Implementation */
  if(pathname == NULL)
  {
    return -1;
  }

  struct inode *node = vfs_lookup(pathname);
  if(node == NULL)
  {
    return -1;
  }

  struct file *f_i2c = (struct file *)malloc(sizeof(struct file));
  if(f_i2c == NULL)
  {
    return -1;
  }

  f_i2c->f_flags = flags;
  f_i2c->f_inode = node;
  f_i2c->f_op = f_i2c->f_inode->i_cdev->ops;

  int fd;
  int ret = pthread_mutex_lock(&g_fd_mutex);
  if(ret != 0)
  {
    return -1;
  }

  for(int i = 0; i < MAX_FDS; i++)
  {
    if(g_fd_table[i] == NULL)
    {
      fd = i;
      g_fd_table[i] = f_i2c;
      f_i2c->f_pos = i;
      break;
    }
  }

  ret = pthread_mutex_unlock(&g_fd_mutex);
  if(ret != 0)
  {
    return -1;
  }

  ret = f_i2c->f_op->open(node, f_i2c);
  if(ret != 0)
  {
    return -1;
  }

  return fd;
}

/**
 * Simulated VFS close() system call.
 *
 * Interface Contract:
 * Invokes driver release operation, deallocates file context, and reclaims file descriptor.
 *
 * @param fd Integer file descriptor to close.
 * @return 0 on success, or -1 on error (setting errno = EBADF).
 */
int sys_cdev_close(int fd)
{
  /* TODO: Author Implementation */
  if((fd < 0) || (fd >= MAX_FDS))
  {
    errno = EBADF;
    return -1;
  }

  if(g_fd_table[fd])
  {
    g_fd_table[fd]->f_op->release(g_fd_table[fd]->f_inode, g_fd_table[fd]);
    free(g_fd_table[fd]);
    g_fd_table[fd] = NULL;
  }
  else
  {
    errno = EBADF;
    return -1;
  }

  return 0;
}

/**
 * Simulated VFS read() system call.
 *
 * @param fd Open file descriptor.
 * @param buf Destination buffer in user address space.
 * @param count Maximum number of bytes to read.
 * @return Number of bytes read on success, or -1 on error (setting errno).
 */
ssize_t sys_cdev_read(int fd, void *buf, size_t count)
{
  if((fd < 0) || (fd >= MAX_FDS))
  {
    errno = EBADF;
    return -1;
  }

  if(g_fd_table[fd] == NULL || g_fd_table[fd]->f_op == NULL || g_fd_table[fd]->f_op->read == NULL)
  {
    errno = EBADF;
    return -1;
  }

  struct file *filp = g_fd_table[fd];
  ssize_t ret = filp->f_op->read(filp, (char *)buf, count, &filp->f_pos);
  if(ret < 0)
  {
    errno = (int)(-ret);
    return -1;
  }

  return ret;
}

/**
 * Simulated VFS write() system call.
 *
 * @param fd Open file descriptor.
 * @param buf Source payload buffer in user address space.
 * @param count Number of bytes to write.
 * @return Number of bytes written on success, or -1 on error (setting errno).
 */
ssize_t sys_cdev_write(int fd, const void *buf, size_t count)
{
  if((fd < 0) || (fd >= MAX_FDS))
  {
    errno = EBADF;
    return -1;
  }

  if(g_fd_table[fd] == NULL || g_fd_table[fd]->f_op == NULL || g_fd_table[fd]->f_op->write == NULL)
  {
    errno = EBADF;
    return -1;
  }

  struct file *filp = g_fd_table[fd];
  ssize_t ret = filp->f_op->write(filp, (const char *)buf, count, &filp->f_pos);
  if(ret < 0)
  {
    errno = (int)(-ret);
    return -1;
  }

  return ret;
}

/**
 * Simulated VFS ioctl() system call.
 *
 * @param fd Open file descriptor.
 * @param cmd ioctl command request.
 * @param arg Command argument.
 * @return 0 on success, or -1 on error (setting errno).
 */
int sys_cdev_ioctl(int fd, unsigned long cmd, unsigned long arg)
{
  if((fd < 0) || (fd >= MAX_FDS))
  {
    errno = EBADF;
    return -1;
  }

  if(g_fd_table[fd] == NULL || g_fd_table[fd]->f_op == NULL ||
     g_fd_table[fd]->f_op->unlocked_ioctl == NULL)
  {
    errno = EBADF;
    return -1;
  }

  struct file *filp = g_fd_table[fd];
  long ret = filp->f_op->unlocked_ioctl(filp, (unsigned int)cmd, arg);
  if(ret < 0)
  {
    errno = (int)(-ret);
    return -1;
  }

  return 0;
}
