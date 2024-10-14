#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    // Open GPIO chip
    //struct gpiod_chip *chip = gpiod_chip_open_by_name("/dev/gpiochip0");
    struct gpiod_chip *chip = gpiod_chip_open_by_name("gpiochip0"); // Make sure the name matches output from gpioinfo
    if (chip == NULL) {
        perror("Failed to open GPIO chip");
        return EXIT_FAILURE;
    }

    // Get GPIO line 17 (input)
    struct gpiod_line *line17 = gpiod_chip_get_line(chip, 17);
    if (line17 == NULL) {
        perror("Failed to get GPIO line 17");
        gpiod_chip_close(chip);  // Clean up the chip before exiting
        return EXIT_FAILURE;
    }

    // Get GPIO line 18 (output)
    struct gpiod_line *line18 = gpiod_chip_get_line(chip, 18);
    if (line18 == NULL) {
        perror("Failed to get GPIO line 18");
        gpiod_chip_close(chip);  // Clean up the chip before exiting
        return EXIT_FAILURE;
    }

    // Request line 17 as input
    if (gpiod_line_request_input(line17, "gpio_input") < 0) {
        perror("Failed to request line 17 as input");
        gpiod_chip_close(chip);  // Clean up the chip before exiting
        return EXIT_FAILURE;
    }

    // Request line 18 as output and set initial value to 0
    if (gpiod_line_request_output(line18, "gpio_output", 0) < 0) {
        perror("Failed to request line 18 as output");
        gpiod_chip_close(chip);  // Clean up the chip before exiting
        return EXIT_FAILURE;
    }

    // Main loop
    while (1) {
        // Read input value from line 17
        int input_value = gpiod_line_get_value(line17);
        if (input_value < 0) {
            perror("Failed to read value from line 17");
            gpiod_chip_close(chip);  // Clean up the chip before exiting
            return EXIT_FAILURE;
        }

        // Set the value on line 18
        if (gpiod_line_set_value(line18, input_value) < 0) {
            perror("Failed to set value on line 18");
            gpiod_chip_close(chip);  // Clean up the chip before exiting
            return EXIT_FAILURE;
        }

        // Sleep for 280 microseconds
        usleep(280);
    }

    // Clean up (although we never reach this point in an infinite loop)
    gpiod_chip_close(chip);
    return EXIT_SUCCESS;
}
