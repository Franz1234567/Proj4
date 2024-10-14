#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int ret;
struct gpiod_line *line17;
struct gpiod_line *line18;
struct gpiod_line_event event;

int main(){
    struct gpiod_chip *chip = gpiod_chip_open_by_name("gpiochip0");

    if(!chip){
    perror("Open chip failed\n");
    return -1;
    }

    line17 = gpiod_chip_get_line(chip, 17);
    line18 = gpiod_chip_get_line(chip, 18);

    if(!line17){
    perror("Get line failed\n");
    ret = -1;
    goto close_chip;
    }
    if(!line18){
    perror("Set line failed\n");
    ret = -1;
    goto close_chip;
    }

    //ret = gpiod_line_request_rising_edge_events(line17, "gpio-test");
    ret = gpiod_line_request_both_edges_events(line17, "gpio-test");
    if(ret < 0){
        perror("Request event notification failed\n");
        ret = -1;
        goto release_line;
    }

    ret = gpiod_line_request_output(line18, "gpio-write", 0);
    if(ret < 0){
        perror("Request output failed\n");
        ret = -1;
        goto release_line;
    }

    while(1) {
        gpiod_line_event_wait(line17, NULL); /* blocking */
        if (gpiod_line_event_read(line17, &event) != 0)
            continue;

    /* should always be a rising event in our example */

        if (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE || event.event_type == GPIOD_LINE_EVENT_FALLING_EDGE){
            int input_value = gpiod_line_get_value(line17);

            if (input_value < 0) {
                perror("Failed to read value from line 17");
                ret = -1;
                goto close_chip;
                return ret;
            }
            gpiod_line_set_value(line18, input_value);
        }
    }
    
    release_line:

    gpiod_line_release(line17);
    gpiod_line_release(line18);

    close_chip:

    gpiod_chip_close(chip);

    return ret;

}