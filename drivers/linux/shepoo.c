#include <linux/module.h>
#include <linux/init.h>

static int __init shepoo_init(void)
{
    printk ("Loading shepoo block device driver.\n");
        return 0;
}

static void __exit shepoo_exit(void)
{
    printk ("Unloading shepoo block device driver..\n");
        return;
}

module_init(shepoo_init);
module_exit(shepoo_exit);

MODULE_LICENSE("Dual MIT/GPL");
