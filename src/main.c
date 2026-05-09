#include "pico/stdlib.h"
#include <stdio.h>

int main() {
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    while (true) {
        printf("Hello, World!\n");
        sleep_ms(500);
    }
}
