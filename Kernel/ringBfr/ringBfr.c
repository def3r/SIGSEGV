#include <asm-generic/errno-base.h>
#include <linux/cdev.h>
#include <linux/container_of.h>
#include <linux/device/class.h>
#include <linux/fs.h>
#include <linux/gfp_types.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>

#define MY_MAJOR 63
#define MY_MAXMINOR 3

struct queue_data {
  int length;
  char *data;
};

struct queue {
  int capacity;
  int size, front;
  struct queue_data *q_data;
};

struct dev_data {
  struct cdev cdev;
  struct queue *q;
};

static int def3r_chardev_uevent(const struct device *dev,
                                struct kobj_uevent_env *env) {
  add_uevent_var(env, "DEVMODE=%#o", 0666);
  return 0;
}

static struct class *def3r_chardev_class = NULL;
struct dev_data devs[MY_MAXMINOR];

static int def3r_open(struct inode *i, struct file *f) {
  struct dev_data *ddata;
  ddata = container_of(i->i_cdev, struct dev_data, cdev);
  f->private_data = ddata;
  return 0;
}

// clang-format off
#define SET_SIZE_OF_QUEUE _IOW('a', 'a', int*)
#define PUSH_DATA         _IOW('a', 'b', struct queue_data *)
#define POP_DATA          _IOR('a', 'c', struct queue_data *)
// clang-format on
static long def3r_ioctl(struct file *f, unsigned int cmd, unsigned long arg) {
  int capacity = 0, rear = 0;
  struct queue_data __user *qdata = 0;
  struct queue_data dummy = {};
  struct dev_data *ddata = (struct dev_data *)f->private_data;

  switch (cmd) {
  case SET_SIZE_OF_QUEUE:
    // kmalloc & kzalloc, allocate 'kernel space heap pages'
    // GFP_KERNEL -> blocking allocation
    // GFP_ATOMIC -> atomic allocation
    if (copy_from_user(&capacity, (int *)arg, sizeof(int)))
      return -EFAULT;
    if (capacity <= 0)
      return -EINVAL;
    if (ddata->q) {
      while (ddata->q->size != 0) {
        kfree(ddata->q->q_data[ddata->q->front].data);
        ddata->q->front = (ddata->q->front + 1) % ddata->q->capacity;
        ddata->q->size--;
      }
      kfree(ddata->q->q_data);
    }
    kfree(ddata->q);
    ddata->q = (struct queue *)kzalloc(sizeof(struct queue), GFP_KERNEL);
    if (!ddata->q)
      return -ENOMEM;
    ddata->q->front = 0;
    ddata->q->size = 0;
    ddata->q->capacity = capacity;
    ddata->q->q_data = (struct queue_data *)kzalloc(
        sizeof(struct queue_data) * capacity, GFP_KERNEL);
    break;

  case PUSH_DATA:
    if (!ddata->q)
      return -EINVAL;
    if (ddata->q->size == ddata->q->capacity)
      return -EINVAL;
    rear = (ddata->q->front + ddata->q->size - 1) % ddata->q->capacity;

    qdata = (struct queue_data __user *)arg;

    // Since copy from user perfoms a shallow copy
    // get the length and user space pointer for string
    if (copy_from_user(&ddata->q->q_data[rear], (struct queue_data *)arg,
                       sizeof(struct queue_data)))
      return -EFAULT;
    printk("PUSH_DATA: d length : %d\n", ddata->q->q_data[rear].length);

    // allocate kernel space buf for string with len
    // use copy_from_user again on the user space char*
    char *buf = kmalloc(ddata->q->q_data[rear].length, GFP_KERNEL);
    if (copy_from_user(buf, ddata->q->q_data[rear].data,
                       ddata->q->q_data[rear].length))
      return -EFAULT;
    ddata->q->q_data[rear].data = buf;
    printk("PUSH_DATA: d data   : %s\n", ddata->q->q_data[rear].data);
    ddata->q->size++;
    break;

  case POP_DATA:
    if (!ddata->q)
      return -EINVAL;
    if (ddata->q->size == 0)
      return -EINVAL;

    qdata = (struct queue_data __user *)arg;

    if (copy_from_user(&dummy, qdata, sizeof(struct queue_data)))
      return -EFAULT;

    if (copy_to_user(&qdata->length, &ddata->q->q_data[ddata->q->front].length,
                     sizeof(int)))
      return -EFAULT;

    if (copy_to_user(dummy.data, ddata->q->q_data[ddata->q->front].data,
                     ddata->q->q_data[ddata->q->front].length))
      return -EFAULT;

    ddata->q->front = (ddata->q->front + 1) % ddata->q->size;
    ddata->q->size--;
    break;

  default:
    return -ENOTTY;
  }

  return 0;
}

static const struct file_operations fops = {
    .open = def3r_open,
    .unlocked_ioctl = def3r_ioctl,
};

int module_initialize(void);
void module_clean(void);

int module_initialize(void) {
  int i, err;

  err = register_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAXMINOR, "def3r");
  if (err)
    return err;

  def3r_chardev_class = class_create("def3r_chardev");
  def3r_chardev_class->dev_uevent = def3r_chardev_uevent;
  for (i = 0; i < MY_MAXMINOR; i++) {
    cdev_init(&devs[i].cdev, &fops);
    devs[i].cdev.owner = THIS_MODULE;
    cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR, i), 1);
    device_create(def3r_chardev_class, NULL, MKDEV(MY_MAJOR, i), NULL,
                  "def3r-%d", i);
  }
  printk("my driver inserted in your kernel!\n");

  return 0;
}

void module_clean(void) {
  int i;
  for (i = 0; i < MY_MAXMINOR; i++) {
    cdev_del(&devs[i].cdev);
    device_destroy(def3r_chardev_class, MKDEV(MY_MAJOR, i));
  }
  unregister_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAXMINOR);
  class_destroy(def3r_chardev_class);
  printk("my driver removed from your kernel!\n");
}

module_init(module_initialize);
module_exit(module_clean);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ring Buffer in kernel space");
