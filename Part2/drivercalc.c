#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// #define NUM_SAMPLES 1000 // Number of samples to measure to measure time

int main(int argc, char *argv[]) {

    if ( argc < 2)
    {
        printf("Usage: %s <number>\n",argv[0]);
        return 1;
    }

    int reference_value = atoi(argv[1]);

    int ret;
    int fd = open("/dev/mydriver", O_RDWR);
    
    int speedbuff;

    // Controller
    // PI Controller parameters
    double Kp = 0.0135;
    double Ti = 1.66;
    double T = 0.01;
    double sum_error = 0;
    double max_speed = 3000;
    //double reference_value = 1500; // Target speed, is given by user in command line
    double u = 0;
    int counter = 0; // to print very seconds

    // Timing variables
    // struct timespec start, end;
    // double time_diff_ns;
    // double jitter[NUM_SAMPLES]; // Array to store time differences (jitter)
    // int sample_index = 0;

    // Start time measurement
    // clock_gettime(CLOCK_MONOTONIC, &start);

    while (1) { // replace by while(sample_index < NUM_SAMPLES){ to measure jitter
        read(fd, &speedbuff, sizeof(speedbuff)); // Get the speed from the module

        double error = reference_value - speedbuff;
        sum_error += error * T;
        u = Kp * error + (Kp / Ti) * sum_error; // contol law
        if (u >= max_speed) {
            sum_error -= error * T;
        }

        if (u > 99){ u = 99;} //limiting the upper bound of the duty cycle
        if (u < 1){ u = 1;} //limiting the lower bound of the duty cycle

        int pwm = (int)u; // give int to kernel, as it has trouble with floating point numbers
        ret = write(fd, &pwm, sizeof(pwm)); // Send the pwm to the module
        counter = counter+1;

        if(counter == 125) // happens ~every second
        {
            printf("speed: %d ", speedbuff);
            printf("u: %d ", (int)u);
            printf("\n");
            
            counter = 0;
        }

        // Measure the time at the end of the iteration
        //clock_gettime(CLOCK_MONOTONIC, &end);

        // Calculate the time difference in nanoseconds
        //time_diff_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
        //jitter[sample_index++] = time_diff_ns;

        // Set start time for next iteration
        //clock_gettime(CLOCK_MONOTONIC, &start);

        usleep(8000);
    }

    // Print jitter data for analysis
    // printf("Jitter (time difference in nanoseconds between iterations):\n");
    // for (int i = 0; i < NUM_SAMPLES; i++) {
    //     printf("%lf\n", jitter[i]);
    // }

    ret = close(fd);

    return 0;
}
