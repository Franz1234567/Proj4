#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/spinlock.h> // For spinlock

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("A simple LKM for a GPIO interrupt");

#define GPIO_17 529 // according to /sys/kernel/debug/gpio
#define GPIO_19 531 // need to check

/** variable contains pin number of interrupt controller to which GPIO 17 is mapped */
unsigned int irq_number;

/* Declare a timer for periodic interrupts */
static struct timer_list timer_1s;

/* Declare a spinlock to protect shared variables */
static spinlock_t lock;

/* All our variables */
int speed = 0;
int count_pulses = 0;

/**
 * @brief Interrupt service routine is called when an interrupt is triggered
 */
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    bool curr_state_A = gpio_get_value(GPIO_17);
    bool curr_state_B = gpio_get_value(GPIO_19); 

    /* Protect the count_pulses variable with a spinlock */
    spin_lock(&lock);
    if (curr_state_A != curr_state_B) { // Encoder rotating clockwise
        count_pulses++; 
    } else { // Encoder rotating counter-clockwise 
        count_pulses--;
    }
    spin_unlock(&lock);

    return IRQ_HANDLED;
}

/**
 * @brief Callback function for the 1-second timer
 */
void timer_1s_callback(struct timer_list *timer)
{
    /* Protect the count_pulses variable with a spinlock */
    spin_lock(&lock);
    speed = count_pulses;
    count_pulses = 0;  // Reset the pulse counter for the next interval
    spin_unlock(&lock);

    printk("Pulses: %d\n", speed);

    /* Restart the timer for another 1 second */
    mod_timer(&timer_1s, jiffies + msecs_to_jiffies(1000));
}

/**
 * @brief This function is called when the module is loaded into the kernel
 */
static int __init ModuleInit(void)
{
    printk("gpio_irq: Loading module...\n");

    /* Initialize the spinlock */
    spin_lock_init(&lock);

    /* Setup the GPIOs */
    if (gpio_request(GPIO_17, "rpi-gpio-17")) {
        printk("Error!\nCannot allocate GPIO 17\n");
        return -1;
    }
    if (gpio_direction_input(GPIO_17)) {
        printk("Error!\nCannot set GPIO 17 to input!\n");
        gpio_free(GPIO_17);
        return -1;
    }

    if (gpio_request(GPIO_19, "rpi-gpio-19") || gpio_direction_input(GPIO_19)) {
        printk("Error!\nCannot allocate or set GPIO 19\n");
        gpio_free(GPIO_17);
        return -1;
    }

    /* Setup the interrupt for GPIO 17 */
    irq_number = gpio_to_irq(GPIO_17);
    if (request_irq(irq_number, gpio_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "my_gpio_irq", NULL) != 0) {
        printk("Error!\nCannot request interrupt nr.: %d\n", irq_number);
        gpio_free(GPIO_17);
        gpio_free(GPIO_19);
        return -1;
    }

    /* Initialize and start the 1-second timer */
    timer_setup(&timer_1s, timer_1s_callback, 0);
    mod_timer(&timer_1s, jiffies + msecs_to_jiffies(1000));

    printk("Done!\nGPIO 17 is mapped to IRQ Nr.: %d\n", irq_number);
    return 0;
}

/**
 * @brief This function is called when the module is removed from the kernel
 */
static void __exit ModuleExit(void)
{
    printk("gpio_irq: Unloading module...\n");
    free_irq(irq_number, NULL);
    gpio_free(GPIO_17);
    gpio_free(GPIO_19);

    /* Stop and free the timer */
    del_timer(&timer_1s);
}

module_init(ModuleInit);
module_exit(ModuleExit);
