#pragma once

#include "hardware/i2c.h"

#include "gps/gps_defines.h"

#define GPS_MSG_CAPACITY 2048

typedef struct gps_config_t {
    i2c_inst_t* i2c;
    uint8_t     sda;
    uint8_t     scl;
    uint8_t     addr;
} gps_config;

typedef struct gps_message_t {
    char     msg[GPS_MSG_CAPACITY];
    uint16_t msg_length;
} gps_message;

typedef struct gps_raw_data_t {
    float    lat;
    float    lon;
    float    alt;
    uint32_t utc;
    uint32_t date;
    float    speed;
} gps_raw_data;

typedef struct gps_lock_info_t {
    uint8_t SVs;
    uint8_t quality;
    float   HDOP;
} gps_lock_info;

typedef struct gps_inst_t {
    gps_config config;

    gps_message   message;
    gps_raw_data  raw_data;
    gps_lock_info lock_info;

    bool (*read)(const struct gps_inst_t*, uint8_t*, size_t, uint8_t*, size_t);
    bool (*write)(const struct gps_inst_t*, uint8_t*, uint8_t*, size_t);
} gps_inst;

bool gps_init(gps_inst* gps, gps_config config);

uint16_t gps_available(const gps_inst* gps);

bool gps_read_buf(const gps_inst* gps, uint8_t* buf, size_t len);

bool gps_read_message(gps_inst* gps);

float gps_convert_decimal(float nmea_value);
float gps_convert_dms(float nmea_value);
