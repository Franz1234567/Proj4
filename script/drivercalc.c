#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// #define NUM_SAMPLES 1000 // Number of samples to measure


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
    double Kp = 0.0135;  // Proportional gain
    double Ti = 1.66;  // Integral time
    double T = 0.01; // Sample time (8ms)
    double sum_error = 0; // Integral of the error
    double max_speed = 3000;
    //double reference_value = 1500; // Target speed 
    double u = 0;    
    int counter = 0;

    // Timing variables
    // struct timespec start, end;
    // double time_diff_ns;
    // double jitter[NUM_SAMPLES]; // Array to store time differences (jitter)
    // int sample_index = 0;

    // Start time measurement
    // clock_gettime(CLOCK_MONOTONIC, &start);
    //sample_index < NUM_SAMPLES
    while (1) {
        read(fd, &speedbuff, sizeof(speedbuff));
        double error = reference_value - speedbuff;
        sum_error += error * T;
        u = Kp * error + (Kp / Ti) * sum_error; // update new speed
        double test = u;
        if (u >= max_speed) {
            sum_error -= error * T;
        }

        if (u > 99){ u = 95;} //limiting the upper bound of the duty cycle
        if (u < 1){ u = 5;} //limiting the lower bound of the duty cycle
        //u = abs(100 - u);

        int pwm = (int)u;
        ret = write(fd, &pwm, sizeof(pwm));
        counter = counter+1;

        if(counter == 100)
        {
            //printf("sum: %e, pmw: %d, testu %f \n",sum_error, pwm,test);
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
