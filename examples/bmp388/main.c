#include "bmp388/bmp388.h"
#include "bmp388/bmp388_defines.h"

#include <pico/stdlib.h>
#include <stdio.h>

#define BMP_I2C_PORT i2c1
#define BMP_ADDR     0x76
#define BMP_SDA      6
#define BMP_SCL      7

bmp_inst bmp;

/* Need to choose OSR (Oversampling), ODR (Data rate), and IIR in such a way to not
 * overload the sensor. High ODR requires lower OSR and IIR.
 */

int main()
{
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(10);
    }

    bmp_config bmp_config_defaults = {
        .i2c  = BMP_I2C_PORT,
        .sda  = BMP_SDA,
        .scl  = BMP_SCL,
        .addr = BMP_ADDR,
    };

    // Setup the I2C communication to the BMP, verify it works, and
    // read in the configuration from the sensor
    if (!bmp_init(&bmp, bmp_config_defaults)) {
        printf("Failed to load BMP\n");
        while (1) { }
    }

    // Oversampling for Pressure and Temperature
    // X2 for pressure
    // X1 for temperature
    bmp_osr_config(&bmp, BMP_OVERSAMPLE_X2, BMP_OVERSAMPLE_X1);

    // Output Data Rate
    // 100 Hz
    bmp_odr_config(&bmp, BMP_ODR_100);

    // Set the coefficient used for the IIR filter
    // Disable the IIR
    bmp_iir_config(&bmp, BMP_IIR_COEF_0);

    // Enable the pressure and temperature sensor
    bmp_pwr_config(&bmp, true, true, BMP_MODE_NORMAL);

    while (true) {
        bmp_read_values(&bmp);
        float altitude = bmp_calc_altitude(bmp.sensor_data.pressure, BMP_SEA_LEVEL);

        printf("%8.1fPa | %8.1fm | %5.3fC\n", bmp.sensor_data.pressure, altitude,
            bmp.sensor_data.temperature);

        sleep_ms(10);
    }
}
