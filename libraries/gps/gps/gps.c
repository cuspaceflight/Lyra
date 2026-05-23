#include "gps.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <hardware/gpio.h>
#include <hardware/uart.h>
#include <pico/platform/compiler.h>

#include "gps/gps_defines.h"
#include "logger/logger.h"

typedef enum nmea_message_t {
    GGA,
    GLL,
    RMC,
    OTHER,
} nmea_type;

#define COMBINE_UINT8_2(H, L) (((uint16_t)(H) << 8) | ((uint16_t)(L)))

bool gps_i2c_read(const gps_inst* gps, uint8_t* reg, size_t reg_len, uint8_t* dst, size_t dst_len)
{
    LOG_VERBOSE("GPS", "%d@%d->%d@%d", reg_len, *reg, dst_len, *dst);

    if (i2c_write_timeout_us(gps->config.i2c, gps->config.addr, reg, reg_len, true, GPS_TIMEOUT_US)
        < 1)
        return false;

    if (i2c_read_timeout_us(gps->config.i2c, gps->config.addr, dst, dst_len, false, GPS_TIMEOUT_US)
        < 1)
        return false;

    return true;
}

bool gps_i2c_write(const gps_inst* gps, uint8_t* reg, uint8_t* data, size_t len)
{
    LOG_VERBOSE("GPS", "%d@%d", len, *reg);

    uint8_t buf[len * 2];

    for (uint8_t i = 0; i < len * 2; i++) {
        buf[i * 2 + 0] = reg[i];
        buf[i * 2 + 1] = data[i];
    }

    if (i2c_write_timeout_us(gps->config.i2c, gps->config.addr, buf, len * 2, false, GPS_TIMEOUT_US)
        < 1)
        return false;

    return true;
}

bool gps_init(gps_inst* gps, gps_config config)
{
    LOG_DEBUG("GPS", "");

    if (gps == NULL)
        return false;

    gps->config = config;

    i2c_init(gps->config.i2c, 100 * 1000);

    gpio_set_function(gps->config.sda, GPIO_FUNC_I2C);
    gpio_set_function(gps->config.scl, GPIO_FUNC_I2C);

    gpio_pull_up(gps->config.sda);
    gpio_pull_up(gps->config.scl);

    if (gps->read == NULL)
        gps->read = gps_i2c_read;

    if (gps->write == NULL)
        gps->write = gps_i2c_write;

    gps->message.msg_length = 0;

    return true;
}

uint16_t gps_available(const gps_inst* gps)
{
    LOG_VERBOSE("GPS", "");

    uint8_t buf[2];
    uint8_t reg = GPS_REG_NUM_BYTES_H;

    gps->read(gps, &reg, 1, buf, 2);

    return COMBINE_UINT8_2(buf[0], buf[1]);
}

bool gps_read_buf(const gps_inst* gps, uint8_t* buf, size_t len)
{
    LOG_VERBOSE("GPS", "");
    uint8_t reg = GPS_REG_DATA;
    return gps->read(gps, &reg, 1, buf, len);
}

int16_t find(const char* str, uint16_t length, uint16_t start, char delim)
{
    for (uint16_t i = start; i < length; i++) {
        if (str[i] == delim) {
            return i;
        }
    }

    return -1;
}

uint16_t split(
    const char* str, size_t str_length, char delim, uint16_t* indices, size_t indices_size)
{
    uint16_t index = 0;
    for (uint16_t i = 0; i < str_length; i++) {
        char c = str[i];

        if (c == delim) {
            indices[index] = i;
            index++;

            if (index >= indices_size)
                return index;
        }
    }

    return index;
}

nmea_type gps_get_nmea_type(gps_inst* gps)
{
    LOG_DEBUG("GPS", "");

#define COMPARE_MSG_3(MSG, STR)                                                                    \
    ((MSG)[0] == '$' && (MSG)[3] == (STR)[0] && (MSG)[4] == (STR)[1] && (MSG)[5] == (STR)[2])

    if (COMPARE_MSG_3(gps->message.msg, "GGA"))
        return GGA;

    if (COMPARE_MSG_3(gps->message.msg, "GGL"))
        return GLL;

    if (COMPARE_MSG_3(gps->message.msg, "RMC"))
        return RMC;

#undef COMPARE_MSG_3

    return OTHER;
}

bool gps_parse_nmea_gll(gps_inst* gps)
{
    LOG_DEBUG("GPS", "");
    uint16_t indices[7];
    uint16_t entries = split(gps->message.msg, gps->message.msg_length, ',', indices, 7);

    if (entries < 7)
        return false;

    gps->raw_data.lat = atof(gps->message.msg + indices[0] + 1);
    gps->raw_data.lat *= (gps->message.msg[indices[1] + 1] == 'S') ? -1 : 1;

    gps->raw_data.lon = atof(gps->message.msg + indices[2] + 1);
    gps->raw_data.lon *= (gps->message.msg[indices[3] + 1] == 'W') ? -1 : 1;

    gps->raw_data.utc = (uint32_t)atof(gps->message.msg + indices[4] + 1);

    return true;
}

bool gps_parse_nmea_gga(gps_inst* gps)
{
    LOG_DEBUG("GPS", "");

    uint16_t indices[13];
    uint16_t entries = split(gps->message.msg, gps->message.msg_length, ',', indices, 13);

    if (entries < 13)
        return false;

    gps->raw_data.utc = (uint32_t)atof(gps->message.msg + indices[0] + 1);

    gps->raw_data.lat = atof(gps->message.msg + indices[1] + 1);
    gps->raw_data.lat *= (gps->message.msg[indices[2] + 1] == 'S') ? -1 : 1;

    gps->raw_data.lon = atof(gps->message.msg + indices[3] + 1);
    gps->raw_data.lon *= (gps->message.msg[indices[4] + 1] == 'W') ? -1 : 1;

    gps->lock_info.quality = atoi(gps->message.msg + indices[5] + 1);
    gps->lock_info.SVs     = atoi(gps->message.msg + indices[6] + 1);
    gps->lock_info.HDOP    = atof(gps->message.msg + indices[7] + 1);

    gps->raw_data.alt = atof(gps->message.msg + indices[8] + 1);

    return true;
}

bool gps_parse_nmea_rmc(gps_inst* gps)
{
    LOG_DEBUG("GPS", "");

    uint16_t indices[11];
    uint16_t entries = split(gps->message.msg, gps->message.msg_length, ',', indices, 11);

    if (entries < 11)
        return false;

    if (gps->message.msg[indices[1] + 1] != 'A')
        return false;

    gps->raw_data.utc = (uint32_t)atof(gps->message.msg + indices[0] + 1);

    gps->raw_data.lat = atof(gps->message.msg + indices[2] + 1);
    gps->raw_data.lat *= (gps->message.msg[indices[3] + 1] == 'S') ? -1 : 1;

    gps->raw_data.lon = atof(gps->message.msg + indices[4] + 1);
    gps->raw_data.lon *= (gps->message.msg[indices[5] + 1] == 'W') ? -1 : 1;

    gps->raw_data.speed = atof(gps->message.msg + indices[6] + 1);

    gps->raw_data.date = atoi(gps->message.msg + indices[8] + 1);

    return true;
}

bool gps_parse_message(gps_inst* gps)
{
    LOG_DEBUG("GPS", "%s", gps->message.msg);

    nmea_type type = gps_get_nmea_type(gps);
    switch (type) {
    case GGA:
        return gps_parse_nmea_gga(gps);
    case GLL:
        return gps_parse_nmea_gll(gps);
    case RMC:
        return gps_parse_nmea_rmc(gps);
    case OTHER:
        break;
    }

    return false;
}

bool gps_read_message(gps_inst* gps)
{
    uint16_t length = gps_available(gps);

    if (length == 0)
        return false;

    LOG_DEBUG("GPS", "");
    uint8_t buf[256];

    if (length > 256) {
        length = 256;
    }

    if (!gps_read_buf(gps, (uint8_t*)(buf), length)) {
        LOG_ERROR("GPS", "Failed to read");
        return false;
    }

    bool processed = false;
    for (size_t i = 0; i < length; i++) {
        char c = buf[i];
        if (c == '\r' || (uint8_t)c == 0xFF || (uint8_t)c == 0x7F)
            continue;

        if (gps->message.msg_length >= GPS_MSG_CAPACITY - 1) {
            gps->message.msg_length = 0;
            continue;
        }

        gps->message.msg[gps->message.msg_length] = c;
        gps->message.msg_length++;

        if (c == '\n') {
            gps->message.msg[gps->message.msg_length - 1] = '\0';

            processed = processed || gps_parse_message(gps);

            gps->message.msg_length = 0;
        }
    }

    return processed;
}

float gps_convert_decimal(float nmea_value)
{
    float degrees = floorf(nmea_value / 100.f);
    float minutes = fmodf(nmea_value, 100.f) / 60.f;

    return degrees + minutes;
}

float gps_convert_dms(float nmea_value)
{
    float decimal = gps_convert_decimal(nmea_value);

    float abs_degrees       = fabsf(decimal);
    float floor_abs_degrees = floorf(abs_degrees);

    float degrees = (decimal > 0 ? 1 : -1) * floor_abs_degrees;
    float minutes = floorf(60 * (abs_degrees - floor_abs_degrees));
    float seconds = 3600 * (abs_degrees - floor_abs_degrees) - 60 * minutes;

    return degrees * 10000 + minutes * 100 + seconds;
}
