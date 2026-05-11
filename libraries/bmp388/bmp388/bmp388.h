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
    float p11;
    float p10;
    float p9;
    float p8;
    float p7;
    float p6;
    float p5;
    float p4;
    float p3;
    float p2;
    float p1;
    float t3;
    float t2;
    float t1;
} bmp_calibration;

typedef struct bmp_config_t {
    i2c_inst_t* i2c;
    uint8_t     sda;
    uint8_t     scl;
    uint8_t     addr;
} bmp_config;

typedef struct bmp_inst_t {
    bmp_calibration calibration;
    bmp_config      config;

    bool (*read)(const struct bmp_inst_t*, uint8_t*, size_t, uint8_t*, size_t);
    bool (*write)(const struct bmp_inst_t*, uint8_t*, uint8_t*, size_t);
} bmp_inst;

bool bmp_init(bmp_inst* bmp, bmp_config config);

bool bmp_pwr_config(
    const bmp_inst* bmp, bool pressure_enable, bool temperature_enable, uint8_t mode);
bool bmp_osr_config(const bmp_inst* bmp, uint8_t osr_p, uint8_t osr_t);
bool bmp_odr_config(const bmp_inst* bmp, uint8_t odr);
bool bmp_iir_config(const bmp_inst* bmp, uint8_t iir_filter);

void bmp_read_configuration(bmp_inst* data);

uint8_t bmp_get_chip_id(const bmp_inst* bmp);

uint32_t bmp_read_temperature_raw(const bmp_inst* bmp);
float    bmp_read_temperature(const bmp_inst* bmp);

uint32_t bmp_read_pressure_raw(const bmp_inst* bmp);
float    bmp_read_pressure(const bmp_inst* bmp, float temperature);

float bmp_calc_altitude(float pressure, float sea_level);
