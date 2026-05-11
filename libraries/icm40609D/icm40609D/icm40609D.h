#pragma once

#include "hardware/spi.h"

#include "icm40609D_defines.h"

typedef struct icm_sensor_data_t {
    float temperature;
    float accel[3];
    float gyro[3];
} icm_sensor_data;

typedef struct icm_sensor_correction_t {
    float accel_ssf;
    float gyro_ssf;
} icm_sensor_correction;

typedef struct icm_config_t {
    spi_inst_t* spi;
    uint8_t     miso;
    uint8_t     mosi;
    uint8_t     sck;
    uint8_t     cs;
} icm_config;

typedef struct icm_inst_t {
    icm_config config;

    icm_sensor_correction sensor_correction;
    icm_sensor_data       sensor_data;

    void (*select)(const struct icm_inst_t*, bool);
    void (*write)(const struct icm_inst_t*, uint8_t, uint8_t*, size_t);
    void (*read)(const struct icm_inst_t*, uint8_t, uint8_t*, size_t);
} icm_inst;

bool icm_init(icm_inst* icm, icm_config config);

void icm_set_bank(const icm_inst* icm, uint8_t bank);

void icm_pwr_mgmt(
    const icm_inst* icm, uint8_t accel_mode, uint8_t gyro_mode, bool idle, bool disable_temp);
void icm_gyro_config(icm_inst* icm, uint8_t fsr, uint8_t odr);
void icm_accel_config(icm_inst* icm, uint8_t fsr, uint8_t odr);

uint8_t icm_get_id(const icm_inst* icm);

void  icm_read_data(icm_inst* icm);
float icm_read_temperature(const icm_inst* icm);
