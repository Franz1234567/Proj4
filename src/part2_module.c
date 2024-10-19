#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("A simple LKM for a gpio interrupt");

#define GPIO_17 529 
// #define GPIO_18 530 was used for testing interrupts on the oscilloscope
#define GPIO_19 531
#define GPIO_13 525 
#define  DEVICE_MAJOR 0  // Requested device node major number or 0 for dynamic allocation
#define  DEVICE_NAME "mydriver" 

static int majorNumber;

unsigned int irq_number; // interrupt for GPIO 17

static struct timer_list timer_1s; // 1s timer for speed
void timer_1s_callback(struct timer_list *timer);

struct hrtimer my_hrtimer400us; // 400 us timer for PWM
struct hrtimer my_hrtimer4us; // 4 us timer for PWM

// All our variables

int speed = 0;
int count_pulses = 0;
int count_pwm = 0;
int count_max_pwm = 0; // intial value 

//Interrupt service routine is called, when interrupt is triggered
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    bool curr_state_A = gpio_get_value(GPIO_17);
    bool curr_state_B = gpio_get_value(GPIO_19); 

    // interrupt occured when PIN 17 linked to C1 changed (rising or falling edge)
    if (curr_state_A != curr_state_B) { // if they are different, it means that the encoder is rotating clockwise
        count_pulses++; 
    } else { // they are the same, it means that the encoder is rotating counter-clockwise 
        count_pulses--;
    }
    return IRQ_HANDLED;
}

// The prototype functions for the character driver 
static int mydriver_open(struct inode *, struct file *);
static int mydriver_release(struct inode *, struct file *);
static ssize_t mydriver_write(struct file *, const char *, size_t, loff_t *);
static ssize_t mydriver_read(struct file *, char *, size_t , loff_t *);

static struct file_operations fops =
{
   .open = mydriver_open,
   .write = mydriver_write,
   .release = mydriver_release,
   .read = mydriver_read,
};

static int mydriver_open(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "mydriver: Device has been opened\n");
   return 0;
}

// Copy data from user space to kernel space
static ssize_t mydriver_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    if (copy_from_user(&count_max_pwm, buffer, len)) {
        printk(KERN_ERR "Failed to copy data from user space\n");
        return -EFAULT;
    }
    return len;
}

// Copy data from kernel space to user space
static ssize_t mydriver_read(struct file *filep, char *buffer, size_t len, loff_t *offset) { 
    if (copy_to_user(buffer, &speed, sizeof(speed))) {
        printk(KERN_ERR "Failed to copy data to user space\n");
        return -EFAULT;
    }
    //printk(KERN_INFO "Data sent to user space\n");
    return sizeof(speed);
}

static int mydriver_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "mydriver: Device successfully closed\n");
   return 0;
}

void timer_1s_callback(struct timer_list *timer)
{
    speed = count_pulses;
    printk("Pulses: %d\n", speed);
    count_pulses = 0;

    printk("%d \n", count_max_pwm);
    mod_timer(&timer_1s, jiffies + msecs_to_jiffies(1000));
}

enum hrtimer_restart my_hrtimer400us_callback(struct hrtimer *timer)
{
    gpio_set_value(GPIO_13, 1);
    count_pwm = 0;

    // Restart the timer 
    hrtimer_forward_now(timer, ktime_set(0, 400 * 1000)); // Set for 400 us
    return HRTIMER_RESTART;
}

enum hrtimer_restart my_hrtimer4us_callback(struct hrtimer *timer)
{
    if (count_pwm != count_max_pwm){
        count_pwm++;
    }
    else{
        gpio_set_value(GPIO_13, 0);
    }
    hrtimer_forward_now(timer, ktime_set(0, 4 * 1000)); // Set for 4 us
    return HRTIMER_RESTART; // Restart the timer
}

// function to be called when the module is loaded
static int __init ModuleInit(void)
{
    printk("gpio_irq: Loading module...\n");

    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0) {
        printk(KERN_ALERT "mydriver failed to register a major number\n");
        return majorNumber;
    }
    else {
        printk(KERN_INFO "mydriver: Registered with major number %d\n", majorNumber);
    }

    /* Setup the GPIO */
    if (gpio_request(GPIO_17, "rpi-gpio-17")) {
        printk("Error!\nCannot allocate GPIO 17\n");
        return -1;
    }

    /* Set GPIO 17 direction */
    if (gpio_direction_input(17)) {
        printk("Error!\nCannot set GPIO 17 to input!\n");
        gpio_free(GPIO_17);
        return -1;
    }

    // if (gpio_request(GPIO_18, "rpi-gpio-18")) {
    //     printk("Error!\nCannot allocate GPIO 18\n");
    //     gpio_free(GPIO_17);
    //     return -1;
    // }

    // if (gpio_direction_output(GPIO_18, 0)) {
    //     printk("Error!\nCannot set GPIO 18 to output!\n");
    //     gpio_free(GPIO_17);
    //     gpio_free(GPIO_18);
    //     return -1;
    // }

    /* Setup GPIO 19 */
    if (gpio_request(GPIO_19, "rpi-gpio-19")) {
        printk("Error!\nCannot allocate GPIO 19\n");
        gpio_free(GPIO_17);
        // gpio_free(GPIO_18);
        return -1;
    }

    /* Set GPIO 19 direction */
    if (gpio_direction_input(GPIO_19)) {
        printk("Error!\nCannot set GPIO 19 to input!\n");
        gpio_free(GPIO_17);
        // gpio_free(GPIO_18);
        gpio_free(GPIO_19);
        return -1;
    }

    if (gpio_request(GPIO_13, "rpi-gpio-18")) {
        printk("Error!\nCannot allocate GPIO 13\n");
        gpio_free(GPIO_17);
        return -1;
    }

    if (gpio_direction_output(GPIO_13, 0)) {
        printk("Error!\nCannot set GPIO 13 to output!\n");
        gpio_free(GPIO_17);
        // gpio_free(GPIO_18);
        gpio_free(GPIO_19);
        return -1;
    }

    /* Setup the interrupt */
    irq_number = gpio_to_irq(GPIO_17);

    if (request_irq(irq_number, gpio_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "my_gpio_irq", NULL) != 0) {
        printk("Error!\nCannot request interrupt nr.: %d\n", irq_number);
        gpio_free(GPIO_17);
        // gpio_free(GPIO_18);
        gpio_free(GPIO_19);
        gpio_free(GPIO_13);
        return -1;
    }

    printk("Done!\n");
    printk("GPIO 17 is mapped to IRQ Nr.: %d\n", irq_number);

    // /* Initialize and start the 1 second timer */
    timer_setup(&timer_1s, timer_1s_callback, 0);
    mod_timer(&timer_1s, jiffies + msecs_to_jiffies(1000));

    // Initialize and start the high-resolution 400us timer
    ktime_t ktime400us = ktime_set(0, 400 * 1000); // 400 us
    hrtimer_init(&my_hrtimer400us, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    my_hrtimer400us.function = my_hrtimer400us_callback;
    hrtimer_start(&my_hrtimer400us, ktime400us, HRTIMER_MODE_REL);

    // Initialize and start the high-resolution 4us timer
    ktime_t ktime4us = ktime_set(0, 4 * 1000); // 4 us
    hrtimer_init(&my_hrtimer4us, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    my_hrtimer4us.function = my_hrtimer4us_callback;
    hrtimer_start(&my_hrtimer4us, ktime4us, HRTIMER_MODE_REL);

    gpio_set_value(GPIO_13,1); // initially set the PWM to high
    return 0;
}

// function to be called when the module is removed, we free everything that we allocated
static void __exit ModuleExit(void)
{
    printk("gpio_irq: Unloading module...\n");
    unregister_chrdev(majorNumber, DEVICE_NAME);
 
    free_irq(irq_number, NULL);
    // gpio_free(GPIO_18);
    gpio_free(GPIO_17);
    gpio_free(GPIO_19);
    gpio_free(GPIO_13);

    /* Stop and free the timers */
    del_timer(&timer_1s);

    hrtimer_cancel(&my_hrtimer400us);
    hrtimer_cancel(&my_hrtimer4us);
}

module_init(ModuleInit);
module_exit(ModuleExit);