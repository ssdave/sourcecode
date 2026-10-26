/**
 * @file i2c_cdev_driver.c
 * @brief I2C Character Device Driver file_operations switch table and handlers.
 *
 * Academic Attribution:
 * Exercise design based on UNIX device driver architectures and the Linux kernel VFS subsystem
 * (OSTEP Chapters 36 & 39, CS:APP Chapter 10, Corbet/Rubini/Kroah-Hartman "Linux Device Drivers",
 * and Linux kernel drivers/i2c/i2c-dev.c).
 * Retained strictly for independent educational self-study.
 */

#include "i2c_cdev.h"

/* ==============================================================================
 * Exercise Skeletons: I2C Driver Switch Table (file_operations)
 * ============================================================================== */

/**
 * Character device open handler for I2C bus controllers.
 *
 * Interface Contract:
 * Initializes per-open session state and binds it to the open file object.
 * Tracks target peripheral slave address and per-session metrics.
 *
 * @param inode Pointer to device inode.
 * @param filp Pointer to open file object.
 * @return 0 on success, or negative error code (-ENOMEM on allocation failure).
 */
int i2c_cdev_open(struct inode *inode, struct file *filp)
{
  if((inode == NULL) || (filp == NULL))
  {
    return -EINVAL;
  }

  i2c_cdev_session_t *session = (i2c_cdev_session_t *)malloc(sizeof(i2c_cdev_session_t));
  if(session == NULL)
  {
    return -ENOMEM;
  }
  memset(session, 0, sizeof(*session));
  session->slave_set = false;
  session->slave_addr = 0;

  filp->private_data = session;
  return 0;
}

/**
 * Character device release handler for I2C bus controllers.
 *
 * Interface Contract:
 * Releases and deallocates per-open session context associated with the file.
 *
 * @param inode Pointer to device inode.
 * @param filp Pointer to open file object.
 * @return 0 on success.
 */
int i2c_cdev_release(struct inode *inode, struct file *filp)
{
  (void)inode;
  if(filp && filp->private_data)
  {
    free(filp->private_data);
    filp->private_data = NULL;
  }
  return 0;
}

/**
 * Character device ioctl handler for I2C configuration and commands.
 *
 * Command Specifications:
 * - I2C_SLAVE: Configures 7-bit peripheral target slave address for the calling session.
 *              Valid address range is 0x00 to 0x7F (returns -EINVAL if > 0x7F).
 * - I2C_GET_STATS: Retrieves bus transfer statistics into user space buffer.
 *                  Requires thread-safe sampling of controller stats and user memory validation.
 * - I2C_RESET_BUS: Executes hardware line release and bus reset under controller lock.
 * - Unknown command: Returns -ENOTTY.
 *
 * @param filp Pointer to open file object.
 * @param cmd ioctl request command identifier.
 * @param arg ioctl command argument (value or user space pointer).
 * @return 0 on success, or negative error code.
 */
long i2c_cdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
  if(filp == NULL || filp->private_data == NULL || filp->f_inode == NULL)
  {
    return -EBADF;
  }

  i2c_cdev_session_t *session = (i2c_cdev_session_t *)filp->private_data;
  i2c_cdev_device_t *dev = (i2c_cdev_device_t *)filp->f_inode->i_cdev->driver_data;
  if(dev == NULL)
  {
    return -ENODEV;
  }

  switch(cmd)
  {
  case I2C_SLAVE:
  {
    if(arg > 0x7F)
    {
      return -EINVAL;
    }
    session->slave_addr = (uint8_t)arg;
    session->slave_set = true;
    return 0;
  }

  case I2C_GET_STATS:
  {
    if(arg == 0)
    {
      return -EFAULT;
    }
    i2c_bus_stats_t stats_copy;
    pthread_mutex_lock(&dev->bus_lock);
    stats_copy = dev->stats;
    pthread_mutex_unlock(&dev->bus_lock);

    if(copy_to_user((void *)arg, &stats_copy, sizeof(stats_copy)) != 0)
    {
      return -EFAULT;
    }
    return 0;
  }

  case I2C_RESET_BUS:
  {
    pthread_mutex_lock(&dev->bus_lock);
    i2c_bus_reset(dev);
    pthread_mutex_unlock(&dev->bus_lock);
    return 0;
  }

  default:
    return -ENOTTY;
  }
}

/**
 * Character device synchronous read handler.
 *
 * Interface Contract & Invariants:
 * - Requires a configured target slave address prior to read (returns -EINVAL if unset).
 * - Enforces serialized hardware access so multiple calling threads do not interleave bus framing.
 * - Enforces user-kernel memory isolation without holding the hardware bus lock during user copies.
 *
 * @param filp Pointer to open file object.
 * @param buf Destination buffer in user address space.
 * @param count Number of bytes requested.
 * @param f_pos File offset pointer (updated on success).
 * @return Number of bytes read (>= 0), or negative error code (-EINVAL, -EFAULT, -EIO).
 */
ssize_t i2c_cdev_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
  if(filp == NULL || filp->private_data == NULL || f_pos == NULL)
  {
    return -EINVAL;
  }

  if(count == 0)
  {
    return 0;
  }

  if(count > MAX_TRANSFER_LEN)
  {
    return -EINVAL;
  }

  if(!is_valid_user_ptr(buf, count))
  {
    return -EFAULT;
  }

  i2c_cdev_session_t *session = (i2c_cdev_session_t *)filp->private_data;
  if(!session->slave_set)
  {
    return -EINVAL;
  }

  i2c_cdev_device_t *dev = (i2c_cdev_device_t *)filp->f_inode->i_cdev->driver_data;
  if(dev == NULL)
  {
    return -ENODEV;
  }

  uint8_t kbuf[MAX_TRANSFER_LEN];
  pthread_mutex_lock(&dev->bus_lock);
  int ret = i2c_hardware_transfer(dev, session->slave_addr, kbuf, count, true);
  if(ret == 0)
  {
    dev->stats.total_transfers++;
    dev->stats.bytes_read += (uint32_t)count;
    session->session_reads += (uint32_t)count;
    *f_pos += (loff_t)count;
  }
  pthread_mutex_unlock(&dev->bus_lock);

  if(ret != 0)
  {
    return -EIO;
  }

  if(copy_to_user(buf, kbuf, count) != 0)
  {
    return -EFAULT;
  }

  return (ssize_t)count;
}

/**
 * Character device synchronous write handler.
 *
 * Interface Contract & Invariants:
 * - Requires a configured target slave address prior to write (returns -EINVAL if unset).
 * - Enforces user-kernel memory isolation, validating input before bus interaction.
 * - Enforces serialized hardware access so concurrent writes do not collide on the bus.
 *
 * @param filp Pointer to open file object.
 * @param buf Source payload buffer in user address space.
 * @param count Number of bytes to transmit.
 * @param f_pos File offset pointer (updated on success).
 * @return Number of bytes written (>= 0), or negative error code (-EINVAL, -EFAULT, -EIO).
 */
ssize_t i2c_cdev_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
  if(filp == NULL || filp->private_data == NULL || f_pos == NULL)
  {
    return -EINVAL;
  }

  if(count == 0)
  {
    return 0;
  }

  if(count > MAX_TRANSFER_LEN)
  {
    return -EINVAL;
  }

  if(!is_valid_user_ptr(buf, count))
  {
    return -EFAULT;
  }

  i2c_cdev_session_t *session = (i2c_cdev_session_t *)filp->private_data;
  if(!session->slave_set)
  {
    return -EINVAL;
  }

  uint8_t kbuf[MAX_TRANSFER_LEN];
  if(copy_from_user(kbuf, buf, count) != 0)
  {
    return -EFAULT;
  }

  i2c_cdev_device_t *dev = (i2c_cdev_device_t *)filp->f_inode->i_cdev->driver_data;
  if(dev == NULL)
  {
    return -ENODEV;
  }

  pthread_mutex_lock(&dev->bus_lock);
  int ret = i2c_hardware_transfer(dev, session->slave_addr, kbuf, count, false);
  if(ret == 0)
  {
    dev->stats.total_transfers++;
    dev->stats.bytes_written += (uint32_t)count;
    session->session_writes += (uint32_t)count;
    *f_pos += (loff_t)count;
  }
  pthread_mutex_unlock(&dev->bus_lock);

  return (ret == 0) ? (ssize_t)count : -EIO;
}

/* ==============================================================================
 * Driver Table Registration Switch Table
 * ============================================================================== */

const struct file_operations g_i2c_fops = {
    .open = i2c_cdev_open,
    .release = i2c_cdev_release,
    .read = i2c_cdev_read,
    .write = i2c_cdev_write,
    .unlocked_ioctl = i2c_cdev_ioctl,
};
