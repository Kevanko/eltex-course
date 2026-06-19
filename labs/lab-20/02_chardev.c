// Задание 20, модуль 2 — символьное устройство (файл в /dev), чтение/запись
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#define DEVNAME "mychardev"
#define BUFSZ 256
static int major;
static char buffer[BUFSZ];
static int dlen;
static ssize_t dev_read(struct file *f, char __user *u, size_t len, loff_t *off){
    if(*off>=dlen) return 0;
    if(len>dlen-*off) len=dlen-*off;
    if(copy_to_user(u, buffer+*off, len)) return -EFAULT;
    *off+=len; return len;
}
static ssize_t dev_write(struct file *f, const char __user *u, size_t len, loff_t *off){
    if(len>BUFSZ) len=BUFSZ;
    if(copy_from_user(buffer,u,len)) return -EFAULT;
    dlen=len; pr_info("mychardev: wrote %zu bytes\n",len); return len;
}
static int dev_open(struct inode *i, struct file *f){ pr_info("mychardev: open\n"); return 0; }
static int dev_release(struct inode *i, struct file *f){ pr_info("mychardev: release\n"); return 0; }
static const struct file_operations fops={ .owner=THIS_MODULE,
    .open=dev_open,.release=dev_release,.read=dev_read,.write=dev_write };
static int __init cd_init(void){
    major=register_chrdev(0,DEVNAME,&fops);
    if(major<0){ pr_err("mychardev: register failed\n"); return major; }
    pr_info("mychardev: loaded, major=%d (mknod /dev/%s c %d 0)\n",major,DEVNAME,major);
    return 0;
}
static void __exit cd_exit(void){ unregister_chrdev(major,DEVNAME); pr_info("mychardev: unloaded\n"); }
module_init(cd_init); module_exit(cd_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Char device module: /dev file, read/write");
