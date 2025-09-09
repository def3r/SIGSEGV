// Resources
// https://linux-kernel-labs.github.io/refs/heads/master/labs/kernel_modules.html
// https://linux-kernel-labs.github.io/refs/heads/master/labs/device_drivers.html
// https://olegkutkov.me/2018/03/14/simple-linux-character-device-driver/

// In a microkernel model, a kernel module is a loadable kernel object file(.ko)
// which extends the kernel functionality at runtime
// When a kernel module is no longer needed, it can be unloaded
// Most device drivers are used in the form of kernel modules
#include <linux/module.h>
// Compiling Kernel Modules
//    1. Module should not be linked to libraries
//    2. Module must be compiled with the same opts as the kernel in which it'll
//	 be loaded.
//
// Standard Compilation Method: kbuild - needs Makefile & Kbuild file
// Also, to generate compile_commands.json for LSP, use $(bear -- make)
//
// Loading & Unloading Modules
//    $ insmod path/to/module.ko # inserts module
//    $ rmmod module.ko          # removes module

// To acces the user space data
// One should not directly dereference a user-space pointer
//    1. May lead to incorrect behaviour
//    2. May lead to a kernel oops
//       (https://en.wikipedia.org/wiki/Linux_kernel_oops)
//    3. Can cause security issues
//
// Proper access involves:
//    put_user, copy_to_user
//    get_user, copy_from_user
#include <linux/uaccess.h>

// Device Drivers
// Two Categories of dev drivers
//    1. Character Device
//        - Slow Devices, small amt of data
//        - System call go directly to dev driver
//    2. Block Device
//        - Large Data vol, search is common
//        - Driver does not work directly with sys calls

// A unique, fixed (static, but may be dynamic) identifier is assoc with device.
// Identifier consists 2 parts:
//    1. Major
//        - Identifies device type
//        - Mostly, identifies the driver
//    2. Minor
//        - Identifies the device
//        - Mostly, identifies the phys devices served by driver
//
//  /proc/device contains loaded devices along with major identifier
#define MY_MAJOR 42
#define MY_MAX_MINORS 5

// Character Device
// represented by: struct cdev
//    Used to register dev in the system
//
// Most Driver operations use:
//    1. struct file_operations
//    2. struct file
//    3. struct inode
//
// Character device drivers receive unaltered system calls made by users over
// device-type files.
// Implementation of a character device driver means implementing the system
// calls specific to files: open, close, read, write, lseek, mmap
#include <linux/cdev.h>

// sysfs class, this adds the device to be listed through
// $ ls -l /dev/mychardev-*
#include <linux/device/class.h>

#include <linux/container_of.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>

struct my_device_data {
  struct cdev cdev;
  size_t size;
  char *buffer;
};

struct my_device_data devs[MY_MAX_MINORS];

static int my_open(struct inode *inode, struct file *file) {
  struct my_device_data *my_data;
  my_data = container_of(inode->i_cdev, struct my_device_data, cdev);
  file->private_data = my_data;
  return 0;
}

static ssize_t my_read(struct file *file, char __user *user_buffer, size_t size,
                       loff_t *offset) {
  struct my_device_data *my_data = (struct my_device_data *)file->private_data;
  ssize_t len = min(my_data->size - *offset, size);

  if (len == 0)
    return 0;

  if (copy_to_user(user_buffer, my_data->buffer + *offset, len))
    return -EFAULT;

  *offset += len;
  return len;
}

static ssize_t my_write(struct file *file, const char __user *user_buffer,
                        size_t size, loff_t *offset) {
  struct my_device_data *my_data = (struct my_device_data *)file->private_data;
  ssize_t len = min(my_data->size - *offset, size);

  if (len <= 0)
    return 0;

  if (copy_from_user(my_data->buffer, user_buffer, len))
    return -EFAULT;

  *offset += len;
  return len;
}

// Physical device control tasks are done by implementing ioctl function
// ioctl	  - this syscall used `Big Kernel Lock`
// unlocked_ioctl - unlocked version
// http://lwn.net/Articles/119652/
//
struct my_ioctl_data {
}; // this is representational data type (see how it is used below)

// A cmd, can be generated manually, but good to use _IOC macro
#define MY_IOCTL_IN _IOC(_IOC_WRITE, 'k', 1, sizeof(struct my_ioctl_data))

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
  // cmd: is the command sent from user space
  // arg: if buffer is fetched, arg value is a ptr to it

  // struct my_device_data *my_data = (struct my_device_data
  // *)file->private_data;
  struct my_ioctl_data mid;

  switch (cmd) {
  case MY_IOCTL_IN:
    if (copy_from_user(&mid, (struct my_ioctl_data *)arg,
                       sizeof(struct my_ioctl_data)))
      return -EFAULT;
    break;
  default:
    return -ENOTTY;
  }

  return 0;
}

const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .read = my_read,
    .write = my_write,
    // .release = my_release,
    .unlocked_ioctl = my_ioctl,
};

static struct class *mychardev_class = NULL;

int module_initialize(void);
void module_clean(void);

int module_initialize(void) {
  int i, err;

  err = register_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAX_MINORS, "mychardev");
  if (err != 0)
    return err;

  // This creates a sysfs class
  mychardev_class = class_create("mychardev");
  for (i = 0; i < MY_MAX_MINORS; i++) {
    cdev_init(&devs[i].cdev, &my_fops);
    devs[i].cdev.owner = THIS_MODULE;
    cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR, i), 1);
    device_create(mychardev_class, NULL, MKDEV(MY_MAJOR, i), NULL,
                  "mychardev-%d", i);
  }

  // use $ dmesg | tail -1
  printk("my driver inserted in your kernel!\n");

  return 0;
}

void module_clean(void) {
  int i;
  for (i = 0; i < MY_MAX_MINORS; i++) {
    cdev_del(&devs[i].cdev);
    device_destroy(mychardev_class, MKDEV(MY_MAJOR, i));
  }
  unregister_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAX_MINORS);
  class_destroy(mychardev_class);
  printk("my driver removed from your kernel!\n");
}

module_init(module_initialize);
module_exit(module_clean);

// This is needed, else the module wont compile lol
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("First device driver");
