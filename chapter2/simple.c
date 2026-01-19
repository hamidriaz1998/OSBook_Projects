#include <asm/param.h>
#include <linux/gcd.h>
#include <linux/hash.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>

unsigned long jiffies_init, jiffies_exit;

static int simple_init(void) {
  printk(KERN_INFO "Loading Kernel Module\n");
  printk(KERN_INFO "GOLDEN_RATIO_PRIME is: %llu\n", GOLDEN_RATIO_PRIME);

  jiffies_init = jiffies;
  printk(KERN_INFO "Value of jiffies in init is: %lu", jiffies_init);

  printk(KERN_INFO "Value of HZ is: %d", HZ);
  return 0;
}

static void simple_exit(void) {
  printk(KERN_INFO "Removing Kernel Module\n");
  printk(KERN_INFO "GCD of 3300 and 24 is: %lu", gcd(3300, 24));

  jiffies_exit = jiffies;
  printk(KERN_INFO "Value of jiffies in exit is: %lu", jiffies_exit);

  unsigned long difference = jiffies_exit - jiffies_init;
  printk(KERN_INFO "Total timer interrupts since loading module: %lu",
         difference);

  printk(KERN_INFO "Total time elapsed since loading module (in seconds): %lu",
         difference / HZ);
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hamid");
MODULE_DESCRIPTION("Simple Module");
