#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/random.h>

#define MAX_DEV 1
#define MAX_MSG_LEN 100

static int chardev_open(struct inode *inode, struct file *file);
static int chardev_release(struct inode *inode, struct file *file);
static long chardev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static ssize_t chardev_read(struct file *file, char __user *buf, size_t len, loff_t *offset);
static ssize_t chardev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset);

static const struct file_operations chardev_fops = {
    .owner      = THIS_MODULE,
    .open       = chardev_open,
    .release    = chardev_release,
    .unlocked_ioctl = chardev_ioctl,
    .read       = chardev_read,
    .write       = chardev_write
};

struct char_device_data {
    struct cdev cdev;
};

static int dev_major = 0;
static struct class *chardev_class = NULL;
static struct char_device_data chardev_data[MAX_DEV];

static int chardev_uevent(struct device *dev, struct kobj_uevent_env *env) {
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static int __init chardev_init(void) {
    int err, i;
    dev_t dev;

    err = alloc_chrdev_region(&dev, 0, MAX_DEV, "chardev");
    dev_major = MAJOR(dev);
    chardev_class = class_create(THIS_MODULE, "chardev");
    chardev_class->dev_uevent = chardev_uevent;

    for (i = 0; i < MAX_DEV; i++) {
        cdev_init(&chardev_data[i].cdev, &chardev_fops);
        chardev_data[i].cdev.owner = THIS_MODULE;
        cdev_add(&chardev_data[i].cdev, MKDEV(dev_major, i), 1);
        device_create(chardev_class, NULL, MKDEV(dev_major, i), NULL, "chardev%d", i);
    }
    printk("CHARDEV INITIALIZED\n");

    return 0;
}

static void __exit chardev_exit(void) {
    int i;

    for (i = 0; i < MAX_DEV; i++) {
        device_destroy(chardev_class, MKDEV(dev_major, i));
    }

    class_unregister(chardev_class);
    class_destroy(chardev_class);
    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);
    printk("CHARDEV EXIT\n");
}

static int chardev_open(struct inode *inode, struct file *file) {
    printk("CHARDEV: Device open\n");
    return 0;
}

static int chardev_release(struct inode *inode, struct file *file) {
    printk("CHARDEV: Device close\n");
    return 0;
}

static long chardev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    printk("CHARDEV: Device ioctl\n");
    return 0;
}

static int power(int num, int times) {
    int i;
    int res = num;
    for (i = 0; i < times - 1; i++) {
        res *= num; 
    }
    return res;
}

static int gen_randnum(void) {
    int res;
    uint8_t bound;
    get_random_bytes(&res, sizeof(res));
    get_random_bytes(&bound, sizeof(bound));
    return res % power(10, bound % 10);
}

static ssize_t chardev_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    int randnum = gen_randnum();
    char data[100] = {0};
    ssize_t bytes = len < (MAX_MSG_LEN-(*offset)) ? len : (MAX_MSG_LEN-(*offset));
    
    sprintf(data, "%d", randnum);
    if (copy_to_user(buf, &data, bytes)) {
        return -EFAULT;
    }

    (*offset) += bytes;
    return bytes;
}

static ssize_t chardev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    return 0;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BrianDoan&LenusNam");

module_init(chardev_init);
module_exit(chardev_exit);