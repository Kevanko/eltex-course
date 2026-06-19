// Задание 20, модуль 3 — procfs: файл в /proc, чтение/запись
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/string.h>
#define PROCNAME "mymodule"
static char buffer[256]="hello from procfs\n";
static struct proc_dir_entry *entry;
static ssize_t p_read(struct file *f, char __user *u, size_t len, loff_t *off){
    return simple_read_from_buffer(u,len,off,buffer,strlen(buffer));
}
static ssize_t p_write(struct file *f, const char __user *u, size_t len, loff_t *off){
    if(len>255) len=255;
    if(copy_from_user(buffer,u,len)) return -EFAULT;
    buffer[len]=0; return len;
}
static const struct proc_ops pops={ .proc_read=p_read, .proc_write=p_write };
static int __init pr_init(void){
    entry=proc_create(PROCNAME,0666,NULL,&pops);
    if(!entry) return -ENOMEM;
    pr_info("procfs: /proc/%s created\n",PROCNAME); return 0;
}
static void __exit pr_exit(void){ proc_remove(entry); pr_info("procfs: /proc/%s removed\n",PROCNAME); }
module_init(pr_init); module_exit(pr_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("procfs module: /proc file, read/write");
