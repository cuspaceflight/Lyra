#include "pico/stdlib.h"
#include <stdio.h>

#include "defines.h"

#include "bmp388/bmp388.h"
#include "icm40609D/icm40609D.h"

bmp_inst bmp;
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

    if (!icm_init(&icm, icm_config_defaults)) {
        printf("Failed to load icm\n");
    }

    icm_pwr_mgmt(&icm, ICM_PWR_LOW_NOISE, ICM_PWR_LOW_NOISE, false, false);
    icm_accel_config(&icm, ICM_ACCEL_FSR_32g, ICM_ACCEL_ODR_2_KHZ);
    icm_gyro_config(&icm, ICM_GYRO_FSR_500, ICM_GYRO_ODR_2_KHZ);

    bmp_config bmp_config_defaults = {
        .i2c  = BMP_I2C_PORT,
        .sda  = BMP_SDA,
        .scl  = BMP_SCL,
        .addr = BMP_ADDR,
    };

    if (!bmp_init(&bmp, bmp_config_defaults)) {
        printf("Failed to load bmp\n");
    }
    bmp_osr_config(&bmp, BMP_OVERSAMPLE_X16, BMP_OVERSAMPLE_X2);
    bmp_odr_config(&bmp, BMP_ODR_100);
    bmp_iir_config(&bmp, BMP_IIR_COEF_7);
    bmp_pwr_config(&bmp, false, true, BMP_MODE_NORMAL);

    sleep_ms(1000);

    while (true) {
        uint8_t id = icm_get_id(&icm);
        icm_read_data(&icm);

        float temp     = bmp_read_temperature(&bmp);
        float pressure = bmp_read_pressure(&bmp, temp);
        float altitude = bmp_calc_altitude(pressure, BMP_SEA_LEVEL);

        printf("%8.1f | %5.3f %5.3f | %+8.6f %+8.6f %+8.6f | %+8.6f %+8.6f %+8.6f\n", pressure,
            temp, icm.sensor_data.temperature, icm.sensor_data.accel[0], icm.sensor_data.accel[1],
            icm.sensor_data.accel[2], icm.sensor_data.gyro[0], icm.sensor_data.gyro[1],
            icm.sensor_data.gyro[2]);
        sleep_ms(50);
    }
}
