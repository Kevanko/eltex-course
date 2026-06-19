// Задание 20, модуль 1 — базовый: init/exit + вывод в лог (pr_info)
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
static int __init hello_init(void){ pr_info("hello: module loaded (init)\n"); return 0; }
static void __exit hello_exit(void){ pr_info("hello: module unloaded (exit)\n"); }
module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eltex student");
MODULE_DESCRIPTION("Base kernel module: init/exit + pr_info");
