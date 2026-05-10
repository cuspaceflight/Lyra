#include "pico/stdlib.h"
#include <stdio.h>

#include "bmp.h"
#include "mpu.h"

bmp_data   bmp;
mpu_config mpu;

int main()
{
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    mpu_defaults(&mpu);
    if (!mpu_init(&mpu)) {
        printf("Failed to load MPU\n");
    }

    mpu_pwr_mgmt(&mpu, MPU_PWR_LOW_NOISE, MPU_PWR_LOW_NOISE, false, false);

    // bmp_init(&bmp);
    // bmp_osr_config(&bmp, BMP_OVERSAMPLE_X16, BMP_OVERSAMPLE_X2);
    // bmp_odr_config(&bmp, BMP_ODR_100);
    // bmp_iir_config(&bmp, BMP_IIR_COEF_7);
    // bmp_pwr_config(&bmp, false, true, BMP_MODE_NORMAL);

    while (true) {
        uint8_t id = mpu_get_id(&mpu);
        // float   temp = mpu_read_temperature(&mpu);
        mpu_read_data(&mpu);

        printf("0x%x | %f | %d %d %d | %d %d %d\n", id, mpu.sensor_data.temperature,
            mpu.sensor_data.accel[0], mpu.sensor_data.accel[1], mpu.sensor_data.accel[2],
            mpu.sensor_data.gyro[0], mpu.sensor_data.gyro[1], mpu.sensor_data.gyro[2]);
        // uint8_t id = bmp_get_chip_id(&bmp);
        // float temp = bmp_read_temperature(&bmp);
        // float pressure = bmp_read_pressure(&bmp, temp);
        //
        // float altitude = bmp_calc_altitude(pressure, BMP_SEA_LEVEL);
        // printf("Hello, World!: 0x%x : %f : %f : %f\n", id, temp, pressure, altitude);
        sleep_ms(500);
    }
}
