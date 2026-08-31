#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

// Метаданные модуля
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Semchik"); 
MODULE_DESCRIPTION("My Hello World Module for Eltex");

// Функция, которая выполняется при загрузке модуля
static int __init hello_init(void)
{
    printk(KERN_INFO "Hello world! I'm Eltex Student\n");
    return 0;    // 0 означает успех
}

// Функция, которая выполняется при выгрузке модуля
static void __exit hello_cleanup(void)
{
    printk(KERN_INFO "Cleaning up module.\n");
}

// Регистрация функций входа и выхода
module_init(hello_init);
module_exit(hello_cleanup);