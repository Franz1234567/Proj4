#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define BUFFER_LENGTH 1024

int main() {
    int ret;
    char out_buffer[BUFFER_LENGTH];

    int fd = open("/dev/mydriver", O_RDWR);
    
    int speedbuff;

    // Controller
    // PI Controller parameters
    double Kp_ = 1.0;  // Proportional gain
    double Ti = 1.0;  // Integral time
    double T = 0.008; // Sample time (8ms)
    double sum_error = 0; // Integral of the error
    double max_speed = 3000;
    double reference_value = 2000; // Target speed 
    double u = 0;    

    while (1) {
        read(fd, &speedbuff, sizeof(speedbuff));
        double error = reference_value - speedbuff;
        sum_error += error * T;
        u = Kp * error + (Kp / Ti) * sum_error; // update new speed
        
        if (u >= max_speed) {
            sum_error -= error * T;
        }

        if (u > 99){ u = 99;} //limiting the upper bound of the duty cycle
        if (u < 1){ u = 1;} //limiting the lower bound of the duty cycle
        u = abs(100 - u);

        int pwm = (int)u;
        ret = write(fd, &pwm, sizeof(pwm));

        usleep(8000);
    }
    ret = close(fd);
    
    return 0;
}
