// Include section
#include <linux/module.h> //  Needed by all modules
#include <linux/version.h> // For Linux version
#include <linux/kernel.h> // Needed for KERN_ALERT
#include <linux/types.h> // Type header in Linux
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h> // Include file for device
#include <linux/cdev.h> // Include file for dev
#include <linux/uaccess.h> // Include file for user access
#include <linux/random.h> // Include file for the random number generator.
// End of Include section

// [Define] Max Minor devices.
#define MAX_DEV 1

// [Define] max message length.
#define MAX_MSG_LEN 100

// Function declaration section.
static int chardev_open(struct inode *inode, struct file *file);
static int chardev_release(struct inode *inode, struct file *file);
static ssize_t chardev_read(struct file *file, char __user *buf, size_t len, loff_t *offset);
// End of Function declaration section.

// [Structure] Initialize file_operations.
static const struct file_operations chardev_fops = {
    .owner      = THIS_MODULE,
    .open       = chardev_open,
    .release    = chardev_release,
    .read       = chardev_read
};

// [Structure] Device data holder, this structure may be extended to hold additional data.
struct char_device_data {
    struct cdev cdev;
};

// global storage for device Major number.
static int dev_major = 0;

// sysfs class structure (sysfs stand for a filesystem for exporting kernel objects).
static struct class *chardev_class = NULL;

// array of chardev_data for.
static struct char_device_data chardev_data[MAX_DEV];

// Configure UDEV variables to set up correct permissions to the character device
static int chardev_uevent(struct device *dev, struct kobj_uevent_env *env) {
    // chmod 666 
    add_uevent_var(env, "DEVMODE=%#o", 0666); // chmod 666 file – all can read and write
    // Return 0
    return 0;
}

// [Implement function] chardev_init: 
// Parameter: None
static int __init chardev_init(void) {
    // Local variables
    int err, i;
    dev_t dev;
    
    // Allocate chardev region and assign Major number.
    err = alloc_chrdev_region(&dev, 0, MAX_DEV, "chardev");
    
    // To get generated Major number we can use MAJOR() macros.
    dev_major = MAJOR(dev);
    
    // create sysfs class.
    chardev_class = class_create(THIS_MODULE, "chardev");
    
    // Allow chardev: read - write call function chardev_uevent above
    chardev_class->dev_uevent = chardev_uevent above;

    // Create necessary number of the devices.
    for (i = 0; i < MAX_DEV; i++) {
        // init new device
        cdev_init(&chardev_data[i].cdev, &chardev_fops);
        chardev_data[i].cdev.owner = THIS_MODULE;
        
        // add device to the system where "i" is a Minor number of the new device.
        cdev_add(&chardev_data[i].cdev, MKDEV(dev_major, i), 1);
        
        // create device node /dev/chardev-x where "x" is "i", equal to the Minor number.
        device_create(chardev_class, NULL, MKDEV(dev_major, i), NULL, "chardev%d", i);
    }
    
    // Print a message announced that chardev initialed.
    printk("CHARDEV INITIALIZED\n");

    // Return zero
    return 0;
}

// [Implement function] chardev_exit: When character device is no longer required it must be properly destroyed
// Parameter: None
// Return: None
static void __exit chardev_exit(void) {
    // Local variable "i" for counting
    int i;
    
    // For each chardev, we will destroy it!
    for (i = 0; i < MAX_DEV; i++) {
        device_destroy(chardev_class, MKDEV(dev_major, i));
    }

    // Call class_unregister function to unregister chardev_class
    class_unregister(chardev_class);
    
    // Call class_destroy function to unregister chardev_class
    class_destroy(chardev_class);
    
    // Call unregister_chrdev_region to unregister chardev region
    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);
    
    // Print a message announced that chardev destroyed.
    printk("CHARDEV EXIT\n");
}

// [Implement function] chardev_open: Assign to syscall "open"
// Parameter: 
// - struct inode *inode: pass data using struct inode and struct file in Linux device driver programming (The main reason is so that your driver can manage more than one device)
// - struct file *file: data
// Return: 0
static int chardev_open(struct inode *inode, struct file *file) {
    printk("CHARDEV: Device open\n");
    return 0;
}

// [Implement function] chardev_open: Assign to syscall "release"
// Parameter: 
// - struct inode *inode: pass data using struct inode and struct file in Linux device driver programming (The main reason is so that your driver can manage more than one device)
// - struct file *file: data
// Return: 0
static int chardev_release(struct inode *inode, struct file *file) {
    printk("CHARDEV: Device close\n");
    return 0;
}

// [Implement function]: power function
// Parameters take 2 param, one is an integer "number", the other is also an integer "times"
// Return: a power of integer number with time complexity O(n) 
static int power(int num, int times) {
    // Local variables
    int i; // for counting
    int res = num; // for result
    
    // for each time from zero to times - 1
    for (i = 0; i < times - 1; i++) {
        res *= num;  // Assigment res varible by multiply it with num variable
    }
    
    // Return result
    return res;
}

// [Implement function]: gen_randnum use for generating a random number
// Parameters: None
// Return: a result which a random 
static int gen_randnum(void) {
    // Initial an integer varibale
    int res;
    
    // Initial an unsigned integer 8 bit variable bound
    uint8_t bound;
    
    // SYNOPSIS for get_random_bytes: void get_random_bytes(void *buf,int nbytes );
    get_random_bytes(&res, sizeof(res)); // Return a random byte - size of res = 4 bytes, and stores them in a buffer - res
    get_random_bytes(&bound, sizeof(bound)); // Return a random byte - size of bound = 8 bits, and stores them in a buffer - bound
    return res % power(10, bound % 10); // Return res mod (10^ (bound mod 10))
}

static ssize_t chardev_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    // get random number
    int randnum = gen_randnum();
    
    // char array data with fully with 0
    char data[100] = {0};
    
    // if len smaller than max message length - offset, then assign it to bytes variable
    ssize_t bytes = len < (MAX_MSG_LEN-(*offset)) ? len : (MAX_MSG_LEN-(*offset));
    
    // Print a random number
    sprintf(data, "%d", randnum);
   
    // unsigned long copy_to_user(void __user *to, const void *from, unsigned long n);
    if (copy_to_user(buf, &data, bytes)) {
        return -EFAULT;
    }

    (*offset) += bytes;
    return bytes;
}

// Module information
MODULE_LICENSE("GPL"); // GPL LICENSE, users can use this module for FREE
MODULE_AUTHOR("BrianDoan&LenusNam"); // Author module

module_init(chardev_init); // Determined what function will be excuted immediately when after the module is inserted the Kernel
module_exit(chardev_exit); // Determined what function will be excuted immediately when after removing module from the Kernel
