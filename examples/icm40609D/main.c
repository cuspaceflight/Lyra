#include <pico/stdlib.h>

#include <stdio.h>

#include "icm40609D/icm40609D.h"

#define ICM_SPI_PORT spi0
#define ICM_MISO     4
#define ICM_MOSI     3
#define ICM_CS       5
#define ICM_SCK      2

icm_inst icm;

int main()
{
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    icm_config icm_config_defaults = {
        .spi  = ICM_SPI_PORT,
        .miso = ICM_MISO,
        .mosi = ICM_MOSI,
        .sck  = ICM_SCK,
        .cs   = ICM_CS,
    };

    // Initialize the ICM
    if (!icm_init(&icm, icm_config_defaults)) {
        printf("Failed to load icm\n");
    }

    // Configure the power mode of the ICM,
    // Low noise for both pressure and temperature
    // Don't be in sleep mode
    // Don't disable the temperature reading
    icm_pwr_mgmt(&icm, ICM_PWR_LOW_NOISE, ICM_PWR_LOW_NOISE, false, false);

    // Configure the accelerometer
    // Range of +- 32g with 2KHz update time
    icm_accel_config(&icm, ICM_ACCEL_FSR_32g, ICM_ACCEL_ODR_2_KHZ);

    // Configure the gyrometer
    // Range of +- 500 DPS with 2KHz update time
    icm_gyro_config(&icm, ICM_GYRO_FSR_500, ICM_GYRO_ODR_2_KHZ);

    while (true) {
        // Read the accelerometer and gyroscope data from the ICM
        icm_read_data(&icm);

        printf("%5.3f | %+8.6f %+8.6f %+8.6f | %+8.6f %+8.6f %+8.6f\n", icm.sensor_data.temperature,
            icm.sensor_data.accel[0], icm.sensor_data.accel[1], icm.sensor_data.accel[2],
            icm.sensor_data.gyro[0], icm.sensor_data.gyro[1], icm.sensor_data.gyro[2]);
    }
}
