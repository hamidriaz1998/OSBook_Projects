#include "linux/fs.h"
#include "linux/jiffies.h"
#include "linux/uaccess.h"
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

#define BUFFER_SIZE 256
#define PROC_NAME "seconds"

unsigned long jiffies_init;

ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count,
                  loff_t *pos);

static struct proc_ops proc_ops = {.proc_read = proc_read};

static int proc_init(void) {
  jiffies_init = jiffies;
  proc_create(PROC_NAME, 0666, NULL, &proc_ops);

  return 0;
}

static void proc_exit(void) { remove_proc_entry(PROC_NAME, NULL); }

ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count,
                  loff_t *pos) {
  int rv = 0;
  char buffer[BUFFER_SIZE];
  static int completed = 0;

  if (completed) {
    completed = 0;
    return 0;
  }

  completed = 1;

  unsigned long elapsed_jiffies = jiffies - jiffies_init;
  unsigned long elapsed_seconds = elapsed_jiffies / HZ;
  unsigned long elapsed_msecs = (elapsed_jiffies * 1000) / HZ;

  rv = snprintf(
      buffer, BUFFER_SIZE,
      "Module loaded at jiffies: %lu\n"
      "Current jiffies: %lu\n"
      "Elapsed jiffies: %lu\n"
      "Elapsed time: %lu seconds (%lu ms)\n",
      jiffies_init, jiffies, elapsed_jiffies, elapsed_seconds, elapsed_msecs);

  unsigned long bytes_not_copied = copy_to_user(usr_buf, buffer, rv);
  if (bytes_not_copied != 0) {
    printk(KERN_INFO "Data was not copied correctly");
  }

  return rv;
}

module_init(proc_init);
module_exit(proc_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hello module");
MODULE_AUTHOR("Hamid");
