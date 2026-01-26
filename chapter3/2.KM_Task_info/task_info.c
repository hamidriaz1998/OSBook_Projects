#include "linux/fs.h"
#include "linux/pid.h"
#include "linux/sched.h"
#include "linux/slab.h"
#include "linux/types.h"
#include "linux/uaccess.h"
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

#define BUFFER_SIZE 1024
#define PROC_NAME "pid"

ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count,
                  loff_t *pos);
ssize_t proc_write(struct file *file, const char __user *usr_buf, size_t count,
                   loff_t *pos);

static struct proc_ops proc_ops = {.proc_read = proc_read,
                                   .proc_write = proc_write};

static long pid = -1;

static int proc_init(void) {
  proc_create(PROC_NAME, 0666, NULL, &proc_ops);

  return 0;
}

static void proc_exit(void) { remove_proc_entry(PROC_NAME, NULL); }

ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count,
                  loff_t *offset) {
  char buffer[BUFFER_SIZE];
  int bytes_written;

  // Write data to buffer
  if (pid == -1) {
    bytes_written = snprintf(buffer, BUFFER_SIZE, "No PID\n");
  } else {
    struct pid *pid_st = find_vpid(pid);
    struct task_struct *task = pid_task(pid_st, PIDTYPE_PID);
    bytes_written = snprintf(buffer, BUFFER_SIZE,
                             "command = [%s]\n"
                             "pid = [%d]\n"
                             "state = [%d]\n",
                             task->comm, task->pid, task->__state);
  }

  int ret = bytes_written;
  if (*offset >= bytes_written ||
      copy_to_user(usr_buf, buffer, bytes_written)) {
    pr_info("copy_to_user failed");
    ret = 0;
  } else {
    pr_info("procfile read: %s\n", file->f_path.dentry->d_name.name);
    *offset += bytes_written;
  }
  return ret;
}

ssize_t proc_write(struct file *file, const char __user *usr_buf, size_t count,
                   loff_t *pos) {
  char *k_mem;

  k_mem = kmalloc(count + 1, GFP_KERNEL);
  if (!k_mem) {
    pr_info("kmalloc failed");
    return -ENOMEM;
  }
  if (copy_from_user(k_mem, usr_buf, count)) {
    kfree(k_mem);
    return -EFAULT;
  }
  pr_info("Received %zu bytes from user space\n", count);
  pr_info("Received this string: %s\n", k_mem);

  if (kstrtol(k_mem, 10, &pid) < 0) {
    kfree(k_mem);
    return -EINVAL;
  }
  pr_info("PID: %ld\n", pid);

  kfree(k_mem);
  return count;
}

module_init(proc_init);
module_exit(proc_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hello module");
MODULE_AUTHOR("Hamid");
