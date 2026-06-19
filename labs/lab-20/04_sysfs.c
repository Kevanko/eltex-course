// Задание 20, модуль 4 — sysfs: атрибут-файл в /sys, чтение/запись
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/string.h>
static char value[256]="hello from sysfs\n";
static struct kobject *kobj;
static ssize_t v_show(struct kobject *k, struct kobj_attribute *a, char *buf){
    return sprintf(buf,"%s",value);
}
static ssize_t v_store(struct kobject *k, struct kobj_attribute *a, const char *buf, size_t n){
    strncpy(value,buf,sizeof(value)-1); value[sizeof(value)-1]=0; return n;
}
static struct kobj_attribute v_attr=__ATTR(myvalue,0664,v_show,v_store);
static int __init s_init(void){
    kobj=kobject_create_and_add("mymodule",kernel_kobj);
    if(!kobj) return -ENOMEM;
    if(sysfs_create_file(kobj,&v_attr.attr)){ kobject_put(kobj); return -ENOMEM; }
    pr_info("sysfs: /sys/kernel/mymodule/myvalue created\n"); return 0;
}
static void __exit s_exit(void){ kobject_put(kobj); pr_info("sysfs: removed\n"); }
module_init(s_init); module_exit(s_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("sysfs module: /sys attribute, read/write");
