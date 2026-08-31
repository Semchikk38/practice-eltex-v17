#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define BUFFER_SIZE 256
#define PROC_FILENAME "my_proc_entry"

static char *msg_buffer;
static size_t msg_len;
static struct proc_dir_entry *proc_entry;

static ssize_t my_proc_read(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
    if (*offp >= msg_len) {
        return 0;
    }

    if (count > msg_len - *offp) {
        count = msg_len - *offp;
    }

    if (copy_to_user(buf, msg_buffer + *offp, count)) {
        return -EFAULT;
    }

    *offp += count;
    
    return count;
}

// Функция записи (когда пользователь делает echo "text" > /proc/my_proc_entry)
static ssize_t my_proc_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
    if (count > BUFFER_SIZE) {
        count = BUFFER_SIZE;
    }

    if (copy_from_user(msg_buffer, buf, count)) {
        return -EFAULT;
    }

    msg_len = count;
    
    if (msg_len < BUFFER_SIZE) {
        msg_buffer[msg_len] = '\0';
    } else {
        msg_buffer[BUFFER_SIZE - 1] = '\0';
    }

    printk(KERN_INFO "my_proc_module: User wrote: %s", msg_buffer);

    return count;
}

static struct proc_ops my_proc_ops = {
    .proc_read = my_proc_read,
    .proc_write = my_proc_write,
};

static int __init my_proc_init(void)
{
    msg_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!msg_buffer) {
        return -ENOMEM;
    }

    strcpy(msg_buffer, "Hello from Kernel! Write to me.\n");
    msg_len = strlen(msg_buffer);

    proc_entry = proc_create(PROC_FILENAME, 0666, NULL, &my_proc_ops);
    if (proc_entry == NULL) {
        kfree(msg_buffer);
        printk(KERN_ERR "my_proc_module: Error creating proc entry\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "my_proc_module: Module loaded. Check /proc/%s\n", PROC_FILENAME);
    return 0;
}

static void __exit my_proc_exit(void)
{
    remove_proc_entry(PROC_FILENAME, NULL);
    kfree(msg_buffer);
    printk(KERN_INFO "my_proc_module: Module unloaded.\n");
}

module_init(my_proc_init);
module_exit(my_proc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Semchik");
MODULE_DESCRIPTION("Proc FS Module without magic numbers and with static vars");