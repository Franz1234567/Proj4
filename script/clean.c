#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>

/* Meta Information */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("A simple LKM to clean GPIO pin 17 on insertion");

#define GPIO_17 529 // GPIO pin 17 according to /sys/kernel/debug/gpio

/**
 * @brief This function is called when the module is loaded into the kernel.
 * It will free GPIO 17 when the module is inserted.
 */
static int __init ModuleInit(void) {
    printk("gpio_clean: Loading module... \n");

    // Free GPIO 17 if it was previously allocated or in use
    if (gpio_is_valid(GPIO_17)) {
        gpio_free(GPIO_17);
        printk("gpio_clean: GPIO 17 has been freed.\n");
    } else {
        printk("gpio_clean: GPIO 17 was not valid or already freed.\n");
    }

    return 0; // Indicate successful module load
}

/**
 * @brief This function is called when the module is removed from the kernel.
 */
static void __exit ModuleExit(void) {
    printk("gpio_clean: Unloading module... \n");
    // No specific cleanup is necessary here as we're only freeing GPIO 17 in ModuleInit
}

module_init(ModuleInit);
module_exit(ModuleExit);
