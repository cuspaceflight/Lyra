#pragma once

#include "hardware/i2c.h"

#define BMP_I2C_PORT i2c1
#define BMP_ADDR     0x76

#define BMP_SDA 6
#define BMP_SCL 7

#define BMP_TIMEOUT_US 1500

#define BMP_REG_CHIP_ID 0x00
#define BMP_REG_ERR     0x02
#define BMP_REG_STATUS  0x03

#define BMP_REG_PRESSURE_MSB  0x06
#define BMP_REG_PRESSURE_LSB  0x05
#define BMP_REG_PRESSURE_XLSB 0x04

#define BMP_REG_TEMPERATURE_XLSB 0x07
#define BMP_REG_TEMPERATURE_LSB  0x08
#define BMP_REG_TEMPERATURE_MSB  0x09

#define BMP_REG_SENSOR_TIME_XLSB 0x0C
#define BMP_REG_SENSOR_TIME_LSB  0x0D
#define BMP_REG_SENSOR_TIME_MSB  0x0E

#define BMP_REG_EVENT      0x10
#define BMP_REG_INT_STATUS 0x11

#define BMP_REG_FIFO_LENGTH_0     0x12
#define BMP_REG_FIFO_LENGTH_1     0x13
#define BMP_REG_FIFO_DATA         0x14
#define BMP_REG_FIFO_WATER_MARK_0 0x15
#define BMP_REG_FIFO_WATER_MARK_1 0x16
#define BMP_REG_FIFO_CONFIG_1     0x17
#define BMP_REG_FIFO_CONFIG_2     0x18

#define BMP_REG_INT_CTRL 0x19
#define BMP_REG_IF_CONF  0x1A
#define BMP_REG_PWR_CTRL 0x1B
#define BMP_REG_OSR      0x1C
#define BMP_REG_ODR      0x1D
#define BMP_REG_CONFIG   0x1F

#define BMP_REG_NVM_P11 0x45
#define BMP_REG_NVM_P10 0x44
#define BMP_REG_NVM_P9H 0x43
#define BMP_REG_NVM_P9L 0x42
#define BMP_REG_NVM_P8  0x41
#define BMP_REG_NVM_P7  0x40
#define BMP_REG_NVM_P6H 0x3F
#define BMP_REG_NVM_P6L 0x3E
#define BMP_REG_NVM_P5H 0x3D
#define BMP_REG_NVM_P5L 0x3C
#define BMP_REG_NVM_P4  0x3B
#define BMP_REG_NVM_P3  0x3A
#define BMP_REG_NVM_P2H 0x39
#define BMP_REG_NVM_P2L 0x38
#define BMP_REG_NVM_P1H 0x37
#define BMP_REG_NVM_P1L 0x36
#define BMP_REG_NVM_T3  0x35
#define BMP_REG_NVM_T2H 0x34
#define BMP_REG_NVM_T2L 0x33
#define BMP_REG_NVM_T1H 0x32
#define BMP_REG_NVM_T1L 0x31

#define BMP_REG_CMD 0x7E

#define BMP_MODE_SLEEP  0b00
#define BMP_MODE_FORCED 0b01
#define BMP_MODE_NORMAL 0b11

#define BMP_OVERSAMPLE_X1  0b000
#define BMP_OVERSAMPLE_X2  0b001
#define BMP_OVERSAMPLE_X4  0b010
#define BMP_OVERSAMPLE_X8  0b011
#define BMP_OVERSAMPLE_X16 0b100
#define BMP_OVERSAMPLE_X32 0b101

#define BMP_ODR_200    0x00
#define BMP_ODR_100    0x01
#define BMP_ODR_50     0x02
#define BMP_ODR_25     0x03
#define BMP_ODR_12p5   0x04
#define BMP_ODR_6p25   0x05
#define BMP_ODR_3p1    0x06
#define BMP_ODR_1p5    0x07
#define BMP_ODR_0p78   0x08
#define BMP_ODR_0p39   0x09
#define BMP_ODR_0p2    0x0A
#define BMP_ODR_0p1    0x0B
#define BMP_ODR_0p05   0x0C
#define BMP_ODR_0p02   0x0D
#define BMP_ODR_0p01   0x0E
#define BMP_ODR_0p006  0x0F
#define BMP_ODR_0p003  0x10
#define BMP_ODR_0p0015 0x11

#define BMP_IIR_COEF_0   0b000
#define BMP_IIR_COEF_1   0b001
#define BMP_IIR_COEF_3   0b010
#define BMP_IIR_COEF_7   0b011
#define BMP_IIR_COEF_15  0b100
#define BMP_IIR_COEF_31  0b101
#define BMP_IIR_COEF_63  0b110
#define BMP_IIR_COEF_127 0b111

#define BMP_SEA_LEVEL 1016.f

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
    bmp_calibration calibration;

    i2c_inst_t* i2c;
    uint8_t     sda;
    uint8_t     scl;
    uint8_t     addr;

    bool (*read)(const struct bmp_config_t*, uint8_t*, size_t, uint8_t*, size_t);
    bool (*write)(const struct bmp_config_t*, uint8_t*, uint8_t*, size_t);
} bmp_config;

void bmp_defaults(bmp_config* config);
bool bmp_init(bmp_config* bmp);

bool bmp_pwr_config(
    const bmp_config* bmp, bool pressure_enable, bool temperature_enable, uint8_t mode);
bool bmp_osr_config(const bmp_config* bmp, uint8_t osr_p, uint8_t osr_t);
bool bmp_odr_config(const bmp_config* bmp, uint8_t odr);
bool bmp_iir_config(const bmp_config* bmp, uint8_t iir_filter);

void bmp_read_configuration(bmp_config* data);

uint8_t bmp_get_chip_id(const bmp_config* bmp);

uint32_t bmp_read_temperature_raw(const bmp_config* bmp);
float    bmp_read_temperature(const bmp_config* bmp);

uint32_t bmp_read_pressure_raw(const bmp_config* bmp);
float    bmp_read_pressure(const bmp_config* bmp, float temperature);

float bmp_calc_altitude(float pressure, float sea_level);
