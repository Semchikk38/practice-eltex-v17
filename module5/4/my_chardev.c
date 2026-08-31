#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mychardev"
#define BUF_LEN 256

static int major_number;
static char message[BUF_LEN] = "Hello from Kernel CharDev! Read me.\n";
static short size_of_message;
static int number_opens = 0;
static struct class *my_class = NULL;
static struct device *my_device = NULL;

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

static int __init chardev_init(void)
{
    printk(KERN_INFO "my_chardev: Initializing module...\n");

    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "my_chardev: Failed to register a major number\n");
        return major_number;
    }
    printk(KERN_INFO "my_chardev: Registered correctly with major number %d\n", major_number);

    my_class = class_create(DEVICE_NAME);
    if (IS_ERR(my_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "my_chardev: Failed to register device class\n");
        return PTR_ERR(my_class);
    }

    my_device = device_create(my_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(my_device)) {
        class_destroy(my_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "my_chardev: Failed to create the device\n");
        return PTR_ERR(my_device);
    }

    size_of_message = strlen(message);
    printk(KERN_INFO "my_chardev: Device created on /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit chardev_exit(void)
{
    device_destroy(my_class, MKDEV(major_number, 0));
    class_destroy(my_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "my_chardev: Module unloaded. Goodbye!\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
    number_opens++;
    printk(KERN_INFO "my_chardev: Device has been opened %d time(s)\n", number_opens);
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;

    if (*offset >= size_of_message) {
        return 0;
    }

    if (len > size_of_message - *offset) {
        len = size_of_message - *offset;
    }

    error_count = copy_to_user(buffer, message + *offset, len);

    if (error_count == 0) {
        *offset += len;
        printk(KERN_INFO "my_chardev: Sent %zu characters to the user\n", len);
        return len;
    } else {
        printk(KERN_ERR "my_chardev: Failed to send %d characters to the user\n", error_count);
        return -EFAULT;
    }
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    memset(message, 0, BUF_LEN);

    if (len > BUF_LEN) {
        len = BUF_LEN;
    }

    if (copy_from_user(message, buffer, len)) {
        return -EFAULT;
    }

    size_of_message = len;
    
    if (message[len-1] != '\n') {
        if (len < BUF_LEN - 1) {
            message[len] = '\n';
            size_of_message++;
        }
    }

    printk(KERN_INFO "my_chardev: Received %zu characters from user: %s", len, message);
    return len;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "my_chardev: Device successfully closed\n");
    return 0;
}

module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Semchik");
MODULE_DESCRIPTION("A simple Linux char device driver for Kernel 7.0");