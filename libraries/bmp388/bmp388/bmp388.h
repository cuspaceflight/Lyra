#pragma once

#include "hardware/i2c.h"

#include "bmp388_defines.h"

typedef struct bmp_calibration_int_t {
    int8_t   p11;
    int8_t   p10;
    int16_t  p9;
    int8_t   p8;
    int8_t   p7;
    uint16_t p6;
    uint16_t p5;
    int8_t   p4;
    int8_t   p3;
    int16_t  p2;
    int16_t  p1;
    int8_t   t3;
    uint16_t t2;
    uint16_t t1;
} bmp_calibration_int;

typedef struct bmp_calibration_t {
    double p11;
    double p10;
    double p9;
    double p8;
    double p7;
    double p6;
    double p5;
    double p4;
    double p3;
    double p2;
    double p1;
    double t3;
    double t2;
    double t1;

    double t_lin;
} bmp_calibration;

typedef struct bmp_config_t {
    i2c_inst_t* i2c;
    uint8_t     sda;
    uint8_t     scl;
    uint8_t     addr;
} bmp_config;

typedef struct bmp_sensor_raw_t {
    uint32_t temp_raw;
    uint32_t press_raw;
} bmp_sensor_raw;

typedef struct bmp_sensor_config_t {
    bool enable_temperature;
    bool enable_pressure;
} bmp_sensor_config;

typedef struct bmp_sensor_data_t {
    float temperature;
    float pressure;
} bmp_sensor_data;

typedef struct bmp_inst_t {
    bmp_config config;

    bmp_calibration calibration;

    bmp_sensor_raw    sensor_raw;
    bmp_sensor_config sensor_config;
    bmp_sensor_data   sensor_data;

    bool (*read)(const struct bmp_inst_t*, uint8_t*, size_t, uint8_t*, size_t);
    bool (*write)(const struct bmp_inst_t*, uint8_t*, uint8_t*, size_t);
} bmp_inst;

bool bmp_init(bmp_inst* bmp, bmp_config config);

void bmp_softreset(const bmp_inst* bmp);

bool bmp_pwr_config(bmp_inst* bmp, bool pressure_enable, bool temperature_enable, uint8_t mode);
bool bmp_osr_config(const bmp_inst* bmp, uint8_t osr_p, uint8_t osr_t);
bool bmp_odr_config(const bmp_inst* bmp, uint8_t odr);
bool bmp_iir_config(const bmp_inst* bmp, uint8_t iir_filter);

void bmp_read_configuration(bmp_inst* data);

uint8_t bmp_get_chip_id(const bmp_inst* bmp);
uint8_t bmp_get_error(const bmp_inst* bmp);

void bmp_read_values(bmp_inst* bmp);

void bmp_read_temperature(bmp_inst* bmp);

void bmp_read_pressure(bmp_inst* bmp);

float bmp_calc_altitude(float pressure, float sea_level);
