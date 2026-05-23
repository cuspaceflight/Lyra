#include <pico/stdlib.h>

#include <stdio.h>

#include "gps/gps.h"

#include "logger/logger.h"

#define GPS_I2C  i2c0
#define GPS_SDA  16
#define GPS_SCL  17
#define GPS_ADDR 0x42

gps_inst gps;

int main()
{
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    gps_config config = {
        .i2c  = GPS_I2C,
        .sda  = GPS_SDA,
        .scl  = GPS_SCL,
        .addr = GPS_ADDR,
    };

    // Initialize the ICM
    if (!gps_init(&gps, config)) {
        printf("Failed to load gps\n");
    }
    uint16_t msg_len;

    while (true) {
        bool processed = gps_read_message(&gps);

        if (processed) {
            LOG_INFO("MAIN", "%d:%d | %+014.06f, %+015.06f | %f", gps.raw_data.date,
                gps.raw_data.utc, gps_convert_dms(gps.raw_data.lat),
                gps_convert_dms(gps.raw_data.lon), gps.raw_data.alt);
        }

        sleep_ms(10);
    }
}
