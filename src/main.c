#include "pico/stdlib.h"
#include <stdio.h>

#include "bmp388.h"
#include "icm40609D.h"

bmp_config bmp;
icm_config icm;

int main()
{
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    icm_defaults(&icm);
    if (!icm_init(&icm)) {
        printf("Failed to load icm\n");
    }

    icm_pwr_mgmt(&icm, ICM_PWR_LOW_NOISE, ICM_PWR_LOW_NOISE, false, false);
    icm_accel_config(&icm, ICM_ACCEL_FSR_32g, ICM_ACCEL_ODR_2_KHZ);
    icm_gyro_config(&icm, ICM_GYRO_FSR_500, ICM_GYRO_ODR_2_KHZ);

    // bmp_init(&bmp);
    // bmp_osr_config(&bmp, BMP_OVERSAMPLE_X16, BMP_OVERSAMPLE_X2);
    // bmp_odr_config(&bmp, BMP_ODR_100);
    // bmp_iir_config(&bmp, BMP_IIR_COEF_7);
    // bmp_pwr_config(&bmp, false, true, BMP_MODE_NORMAL);

    while (true) {
        uint8_t id = icm_get_id(&icm);
        // float   temp = icm_read_temperature(&icm);
        icm_read_data(&icm);

        printf("0x%x | %5.3f | %+8.6f %+8.6f %+8.6f | %+8.6f %+8.6f %+8.6f\n", id,
            icm.sensor_data.temperature, icm.sensor_data.accel[0], icm.sensor_data.accel[1],
            icm.sensor_data.accel[2], icm.sensor_data.gyro[0], icm.sensor_data.gyro[1],
            icm.sensor_data.gyro[2]);
        // uint8_t id = bmp_get_chip_id(&bmp);
        // float temp = bmp_read_temperature(&bmp);
        // float pressure = bmp_read_pressure(&bmp, temp);
        //
        // float altitude = bmp_calc_altitude(pressure, BMP_SEA_LEVEL);
        // printf("Hello, World!: 0x%x : %f : %f : %f\n", id, temp, pressure, altitude);
        sleep_ms(50);
    }
}
