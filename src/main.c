#include "pico/stdlib.h"
#include <stdio.h>

#include "bmp.h"

bmp_data bmp;

int main() {
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    bmp_init(&bmp);
    bmp_osr_config(&bmp, BMP_OVERSAMPLE_X16, BMP_OVERSAMPLE_X2);
    bmp_odr_config(&bmp, BMP_ODR_100);
    bmp_iir_config(&bmp, BMP_IIR_COEF_7);
    bmp_pwr_config(&bmp, false, true, BMP_MODE_NORMAL);

    while (true) {
        uint8_t id = bmp_get_chip_id(&bmp);
        float temp = bmp_read_temperature(&bmp);
        float pressure = bmp_read_pressure(&bmp, temp);

        float altitude = bmp_calc_altitude(pressure, BMP_SEA_LEVEL);

        printf("Hello, World!: 0x%x : %f : %f : %f\n", id, temp, pressure, altitude);
        sleep_ms(500);
    }
}
