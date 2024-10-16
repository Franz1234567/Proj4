#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

/* Meta Information */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("A simple LKM for a gpio interrupt");

#define GPIO_17 529 // according to /sys/kernel/debug/gpio
#define GPIO_18 530 // need to check

/** variable contains pin number o interrupt controller to which GPIO 17 is mapped to */
unsigned int irq_number;

/**
 * @brief Interrupt service routine is called, when interrupt is triggered
 */

static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
 //printk("gpio_irq: Interrupt was triggered and ISR was called!\n");
    int value;

    value = gpio_get_value(GPIO_17);
    gpio_set_value(GPIO_18, value);
    return IRQ_HANDLED;


 return IRQ_HANDLED;
}

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
 static int __init ModuleInit(void) {
 printk("qpio_irq: Loading module... ");

 /* Setup the gpio */
 if(gpio_request(GPIO_17, "rpi-gpio-17")) {
 printk("Error!\nCan not allocate GPIO 17\n");
 return -1;
 }

 /* Set GPIO 17 direction */
 if(gpio_direction_input(17)) {
 printk("Error!\nCan not set GPIO 17 to input!\n");
 gpio_free(GPIO_17);
 return -1;
 }

if (gpio_request(GPIO_18, "rpi-gpio-18")) {
    printk("Error!\nCannot allocate GPIO 18\n");
    gpio_free(GPIO_17);
    return -1;
}

if (gpio_direction_output(GPIO_18, 0)) {
    printk("Error!\nCannot set GPIO 18 to output!\n");
    gpio_free(GPIO_17);
    gpio_free(GPIO_18);
    return -1;
}

 /* Setup the interrupt */
 irq_number = gpio_to_irq(GPIO_17);

//  if(request_irq(irq_number, gpio_irq_handler, IRQF_TRIGGER_RISING, "my_gpio_irq", NULL) != 0){
//  printk("Error!\nCan not request interrupt nr.: %d\n", irq_number);
//  gpio_free(GPIO_17);
//  return -1;
//  }

 if (request_irq(irq_number, gpio_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "my_gpio_irq", NULL) != 0) {
    printk("Error!\nCannot request interrupt nr.: %d\n", irq_number);
    gpio_free(GPIO_17);
    return -1;
}

 printk("Done!\n");
 printk("GPIO 17 is mapped to IRQ Nr.: %d\n", irq_number);

 return 0;
}

/**

 * @brief This function is called, when the module is removed from the kernel

 */

static void __exit ModuleExit(void) {
 printk("gpio_irq: Unloading module... ");
 free_irq(irq_number, NULL);
 gpio_free(GPIO_18);
 gpio_free(GPIO_17);
}


module_init(ModuleInit);
module_exit(ModuleExit);


