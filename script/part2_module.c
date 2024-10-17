#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/fs.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("A simple LKM for a gpio interrupt");

#define GPIO_17 529 // according to /sys/kernel/debug/gpio
#define GPIO_18 530 // need to check
#define GPIO_19 531 // need to check
#define GPIO_13 525 // need to check
#define  DEVICE_MAJOR 0         ///< Requested device node major number or 0 for dynamic allocation
#define  DEVICE_NAME "mydriver"   ///< In this implementation, the device name has nothing to do with the name of the device in /dev. You must use mknod to create the device node in /dev

static int   majorNumber;        ///< Stores the device number -- determined automatically

/** variable contains pin number of interrupt controller to which GPIO 17 is mapped */
unsigned int irq_number;

/* Declare two timers for periodic interrupts */
// static struct timer_list timer_8ms;
static struct timer_list timer_1s;
// static struct timer_list timer_400us;
// static struct timer_list timer_4us;

/* Function prototypes for timer callbacks */
// void timer_8ms_callback(struct timer_list *timer);
void timer_1s_callback(struct timer_list *timer);
// void timer_400us_callback(struct timer_list *timer);
// void timer_4us_callback(struct timer_list *timer);

// All our variables

// bool last_state_A;
// bool last_state_B;
// bool curr_state_A;
// bool curr_state_B;

int speed = 0;
int actual_value = 0;
int count_pulses = 0;
int count_pwm = 0;
int count_max_pwm = 100;


// Controller
// PI Controller parameters
// double Kp_ = 1.0;  // Proportional gain
// double Ti = 1.0;  // Integral time
// double T = 0.008; // Sample time (8ms)
// double sum_error = 0; // Integral of the error
// double max_speed = 3000;
// double reference_value = 2000; // Target speed 
// double u = 0;

static int pin18_state = 0;

/**
 * @brief Interrupt service routine is called, when interrupt is triggered
 */
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    // printk("gpio_irq: Interrupt was triggered and ISR was called!\n");
    // sleep for 50 us
    // usleep_range(50, 55);
    // pin18_state = !pin18_state;
    // gpio_set_value(GPIO_18, pin18_state);

    // int gpio_value = gpio_get_value(GPIO_17);
    // printk("GPIO 17 value: %d\n", gpio_value);

    // int gpio17_value;
    // int gpio19_value;

    //udelay(50);

    bool curr_state_A = gpio_get_value(GPIO_17);
    bool curr_state_B = gpio_get_value(GPIO_19); 

    // interrupt occured when PIN 17 linked to C1 changed (rising or falling edge)
    if (curr_state_A != curr_state_B) { // if they are different, it means that the encoder is rotating clockwise
        count_pulses++; 
    } else { // they are the same, it means that the encoder is rotating counter-clockwise 
        count_pulses--;
    }

    return IRQ_HANDLED;


    
    // if (last_state_A == last_state_B) {
    //     gpio17_value = gpio_get_value(GPIO_17);
    //     curr_state_A = (gpio17_value == 0);
    //     if (curr_state_A != last_state_A) {
    //         count_pulses++;
    //         // gpio_set_value(GPIO_18, curr_state_A);
    //     } else {
    //         gpio19_value = gpio_get_value(GPIO_19);
    //         curr_state_B = (gpio19_value == 0);
    //     }
    // } else {
    //     gpio17_value = gpio_get_value(GPIO_17);
    //     curr_state_A = (gpio17_value == 0);
    //     if (curr_state_A != last_state_A) {
    //         count_pulses--;
    //         // gpio_set_value(GPIO_18, curr_state_A);
    //     } else {
    //         gpio19_value = gpio_get_value(GPIO_19);
    //         curr_state_B = (gpio19_value == 0);
    //     }
    // }

    // last_state_A = curr_state_A;
    // last_state_B = curr_state_B;
}

/**
 * @brief Callback function for the 8ms timer
 */

// The prototype functions for the character driver -- must come before the struct definition
static int     mydriver_open(struct inode *, struct file *);
static int     mydriver_release(struct inode *, struct file *);
static ssize_t mydriver_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops =
{
   .open = mydriver_open,
   .write = mydriver_write,
   .release = mydriver_release,
};

static int mydriver_open(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "mydriver: Device has been opened\n");
   return 0;
}

static ssize_t mydriver_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    //printk(KERN_INFO "mydriver: %zu \n", buffer);

    if (copy_from_user(count_max_pwm, buffer, len)) {
        printk(KERN_ERR "Failed to copy data from user space\n");
        return -EFAULT;
    }

    printk(KERN_INFO "Data received from user space: %s\n", buffer);
    return len;
}

static ssize_t mydriver_read(struct file *filep, const char __user *buffer, size_t len, loff_t *offset) {
    // Copy data from kernel space to user space
    if (copy_to_user(buffer, speed, sizeof(speed))) {
        printk(KERN_ERR "Failed to copy data to user space\n");
        return -EFAULT;
    }
    printk(KERN_INFO "Data sent to user space\n");
    return sizeof(speed);
}

static int mydriver_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "mydriver: Device successfully closed\n");
   return 0;
}

 void timer_8ms_callback(struct timer_list *timer) // Pi controller logic is here
 {
     actual_value = speed;
//     // gives speed to user space via input buffer
//     // reads from user space count_max_pwm 

    mod_timer(&timer_8ms, jiffies + msecs_to_jiffies(8));

    // double error = reference_value - actual_value;
    // sum_error_priv += error * T_priv;
    // u = Kp_priv * error + (Kp_priv / Ti_priv) * sum_error_priv; // update new speed
    
    // if (u >= max_speed) {
    //     sum_error_priv -= error * T_priv;
    // }

    // if (u > 99){ u = 99;} //limiting the upper bound of the duty cycle
    // if (u < 1){ u = 1;} //limiting the lower bound of the duty cycle

    // Restart the timer for another 8 ms
 }

void timer_400us_callback(struct timer_list *timer)
{
    gpio_set_value(GPIO_13, 0);
    count_pwm = 0;
    mod_timer(&timer_400us, jiffies + usecs_to_jiffies(400));
}

void timer_4us_callback(struct timer_list *timer)
{
    if (count_pwm != count_max_pwm){
        count_pwm++;
    }
    else{
        gpio_set_value(GPIO_13, 1);
    }
    mod_timer(&timer_4us, jiffies + usecs_to_jiffies(4));
}

/**
 * @brief Callback function for the 1 second timer
 */

void timer_1s_callback(struct timer_list *timer)
{
    //printk("Timer 1s: Interrupt triggered\n");
    /* Restart the timer for another 1 second */
    speed = count_pulses;
    printk("Pulses: %d\n", speed);
    count_pulses = 0;
    mod_timer(&timer_1s, jiffies + msecs_to_jiffies(1000));
}

/**
 * @brief This function is called when the module is loaded into the kernel
 */
static int __init ModuleInit(void)
{
    printk("gpio_irq: Loading module...\n");

    //printk(KERN_INFO "mydriver: Hello %s from the RPi LKM!\n", name);
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

    /* Setup GPIO 19 */
    if (gpio_request(GPIO_19, "rpi-gpio-19")) {
        printk("Error!\nCannot allocate GPIO 19\n");
        gpio_free(GPIO_17);
        gpio_free(GPIO_18);
        return -1;
    }

    /* Set GPIO 19 direction */
    if (gpio_direction_input(GPIO_19)) {
        printk("Error!\nCannot set GPIO 19 to input!\n");
        gpio_free(GPIO_17);
        gpio_free(GPIO_18);
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
        gpio_free(GPIO_18);
        gpio_free(GPIO_19);
        return -1;
    }

    /* Setup the interrupt */
    irq_number = gpio_to_irq(GPIO_17);

    if (request_irq(irq_number, gpio_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "my_gpio_irq", NULL) != 0) {
        printk("Error!\nCannot request interrupt nr.: %d\n", irq_number);
        gpio_free(GPIO_17);
        gpio_free(GPIO_18);
        gpio_free(GPIO_19);
        gpio_free(GPIO_13);
        return -1;
    }

    //last_state_A = (gpio_get_value(GPIO_17) == 0);
    //last_state_B = (gpio_get_value(GPIO_19) == 0);

    printk("Done!\n");
    printk("GPIO 17 is mapped to IRQ Nr.: %d\n", irq_number);

    // /* Initialize and start the 8 ms timer */
    // timer_setup(&timer_8ms, timer_8ms_callback, 0);
    // mod_timer(&timer_8ms, jiffies + msecs_to_jiffies(8));

    // /* Initialize and start the 1 second timer */
    timer_setup(&timer_1s, timer_1s_callback, 0);
    mod_timer(&timer_1s, jiffies + msecs_to_jiffies(1000));

    // /* Initialize and start the 400 us timer */
    // timer_setup(&timer_400us, timer_400us_callback, 0);
    // mod_timer(&timer_400us, jiffies + usecs_to_jiffies(400));

    // /* Initialize and start the 4 us timer */
    // timer_setup(&timer_4us, timer_4us_callback, 0);
    // mod_timer(&timer_4us, jiffies + usecs_to_jiffies(4));

    return 0;
}

/**
 * @brief This function is called when the module is removed from the kernel
 */
static void __exit ModuleExit(void)
{
    printk("gpio_irq: Unloading module...\n");
    unregister_chrdev(majorNumber, DEVICE_NAME);
 
    free_irq(irq_number, NULL);
    gpio_free(GPIO_18);
    gpio_free(GPIO_17);
    gpio_free(GPIO_19);
    gpio_free(GPIO_13);

    /* Stop and free the timers */
    // del_timer(&timer_8ms);
    del_timer(&timer_1s);
    // del_timer(&timer_400us);
    // del_timer(&timer_4us);
}

module_init(ModuleInit);
module_exit(ModuleExit);
