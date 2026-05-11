#include "pico/stdlib.h"
#include <stdio.h>

#include "defines.h"

#include "sx126X/sx126X.h"

lora_inst lora;

int main()
{
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    lora_config lora_config_defaults = {
        .spi      = LORA_SPI_PORT,
        .miso     = LORA_MISO,
        .mosi     = LORA_MOSI,
        .sck      = LORA_SCK,
        .cs       = LORA_CS,
        .rst      = LORA_RST,
        .busy     = LORA_BUSY,
        .freq     = 868E6,
        .syncword = 0x3444,
    };

    if (!lora_init(&lora, lora_config_defaults)) {
        printf("Failed to initialize LoRa");
    }

    lora_set_modulation_params(&lora, LORA_SF_10, LORA_BW_125, LORA_CR_4_5, false);
    // lora_set_packet_params(&lora, 0x00, false, 0x00, true, false);
    lora_set_tx_params(&lora, 1, LORA_RAMP_TIME_200U);
    lora_set_dio_irq_params(
        &lora, LORA_IRQ_TX_DONE | LORA_IRQ_TIMEOUT, LORA_IRQ_TX_DONE | LORA_IRQ_TIMEOUT, 0, 0);

    bool ready_to_send = true;

    while (true) {
        const uint8_t message[] = "Hello, World";
        const size_t  len       = 13;

        if (ready_to_send) {
            ready_to_send = false;
            printf("Sending: '%s'\n", message);

            lora_write_tx_message(&lora, message, len);
            lora_set_packet_params(&lora, 0x00, false, len, true, false);
            lora_tx(&lora, 1000);
        }

        printf("%d\n", lora_get_irq_status(&lora));

        if ((lora_get_irq_status(&lora) & LORA_IRQ_TX_DONE) != 0) {
            ready_to_send = true;
            lora_clear_irq_status(&lora, LORA_IRQ_TX_DONE);
            printf("Ready to send\n");
        }

        sleep_ms(500);
    }
}

// #include "bmp388/bmp388.h"
// #include "icm40609D/icm40609D.h"

// bmp_inst bmp;
// icm_inst icm;
//
// int main()
// {
//     stdio_init_all();
//
//     while (!stdio_usb_connected()) {
//         sleep_ms(100);
//     }
//
//     icm_config icm_config_defaults = {
//         .spi  = ICM_SPI_PORT,
//         .miso = ICM_MISO,
//         .mosi = ICM_MOSI,
//         .sck  = ICM_SCK,
//         .cs   = ICM_CS,
//     };
//
//     if (!icm_init(&icm, icm_config_defaults)) {
//         printf("Failed to load icm\n");
//     }
//
//     icm_pwr_mgmt(&icm, ICM_PWR_LOW_NOISE, ICM_PWR_LOW_NOISE, false, false);
//     icm_accel_config(&icm, ICM_ACCEL_FSR_32g, ICM_ACCEL_ODR_2_KHZ);
//     icm_gyro_config(&icm, ICM_GYRO_FSR_500, ICM_GYRO_ODR_2_KHZ);
//
//     bmp_config bmp_config_defaults = {
//         .i2c  = BMP_I2C_PORT,
//         .sda  = BMP_SDA,
//         .scl  = BMP_SCL,
//         .addr = BMP_ADDR,
//     };
//
//     if (!bmp_init(&bmp, bmp_config_defaults)) {
//         printf("Failed to load bmp\n");
//     }
//     bmp_osr_config(&bmp, BMP_OVERSAMPLE_X16, BMP_OVERSAMPLE_X2);
//     bmp_odr_config(&bmp, BMP_ODR_100);
//     bmp_iir_config(&bmp, BMP_IIR_COEF_7);
//     bmp_pwr_config(&bmp, false, true, BMP_MODE_NORMAL);
//
//     sleep_ms(1000);
//
//     while (true) {
//         uint8_t id = icm_get_id(&icm);
//         icm_read_data(&icm);
//
//         float temp     = bmp_read_temperature(&bmp);
//         float pressure = bmp_read_pressure(&bmp, temp);
//         float altitude = bmp_calc_altitude(pressure, BMP_SEA_LEVEL);
//
//         printf("%8.1f | %5.3f %5.3f | %+8.6f %+8.6f %+8.6f | %+8.6f %+8.6f %+8.6f\n", pressure,
//             temp, icm.sensor_data.temperature, icm.sensor_data.accel[0],
//             icm.sensor_data.accel[1], icm.sensor_data.accel[2], icm.sensor_data.gyro[0],
//             icm.sensor_data.gyro[1], icm.sensor_data.gyro[2]);
//         sleep_ms(50);
//     }
// }
