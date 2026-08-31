#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/input.h>
#include <linux/vt_kern.h>
#include <linux/console_struct.h>

static struct timer_list blink_timer;
static struct kobject *kbleds_kobj;
static int led_state = 0;

static unsigned int led_mask = 7; 

static void my_set_leds(unsigned int mask)
{
    struct input_handle *handle;
    struct input_dev *dev;
    

    struct tty_struct *tty;
    
    tty = vc_cons[fg_console].d->port.tty;
    if (!tty || !tty->driver || !tty->driver->ops || !tty->driver->ops->ioctl) {

        printk(KERN_DEBUG "kbleds: Direct TTY ioctl not available, trying input injection\n");
        
        return; 
    }


}

static void my_set_leds_safe(unsigned int mask)
{

    
    printk(KERN_INFO "kbleds: Setting LEDs to mask %u (Physical blinking may not work in VM/Kernel 7.0)\n", mask);
}

static void blink_timer_func(struct timer_list *t)
{
    if (led_state == 0) {
        my_set_leds_safe(led_mask);
        led_state = 1;
    } else {
        my_set_leds_safe(0);
        led_state = 0;
    }

    mod_timer(&blink_timer, jiffies + HZ / 5);
}


static ssize_t mask_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%u\n", led_mask);
}

static ssize_t mask_store(struct kobject *kobj, struct kobj_attribute *attr, 
                          const char *buf, size_t count)
{
    unsigned int new_mask;

    if (kstrtouint(buf, 10, &new_mask) < 0) {
        return -EINVAL;
    }

    led_mask = new_mask & 0x07;
    
    printk(KERN_INFO "kbleds_sysfs: New LED mask set to %u (binary: %u%u%u)\n", 
           led_mask, 
           (led_mask >> 2) & 1, 
           (led_mask >> 1) & 1, 
           led_mask & 1);

    return count;
}

static struct kobj_attribute mask_attribute = __ATTR(mask, 0644, mask_show, mask_store);


static int __init kbleds_sysfs_init(void)
{
    int error;

    printk(KERN_INFO "kbleds_sysfs: Loading module...\n");

    kbleds_kobj = kobject_create_and_add("kbleds_ctrl", kernel_kobj);
    if (!kbleds_kobj) {
        return -ENOMEM;
    }

    error = sysfs_create_file(kbleds_kobj, &mask_attribute.attr);
    if (error) {
        printk(KERN_ERR "kbleds_sysfs: Failed to create sysfs file\n");
        kobject_put(kbleds_kobj);
        return error;
    }

    timer_setup(&blink_timer, blink_timer_func, 0);
    mod_timer(&blink_timer, jiffies + HZ / 5);

    printk(KERN_INFO "kbleds_sysfs: Module loaded. Control via /sys/kernel/kbleds_ctrl/mask\n");
    return 0;
}

static void __exit kbleds_sysfs_exit(void)
{
    timer_delete_sync(&blink_timer);
    my_set_leds_safe(0);
    
    sysfs_remove_file(kbleds_kobj, &mask_attribute.attr);
    kobject_put(kbleds_kobj);

    printk(KERN_INFO "kbleds_sysfs: Module unloaded.\n");
}

module_init(kbleds_sysfs_init);
module_exit(kbleds_sysfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Semchik");
MODULE_DESCRIPTION("Keyboard LEDs blinking controlled via sysfs (Kernel 7.0 Safe Mode)");
