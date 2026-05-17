#include "bmp388.h"

#include "bmp388_defines.h"

#include "logger/logger.h"

#include "pico/stdlib.h"
#include <math.h>

#define COMBINE_UINT8_2(hi, lo) (((uint16_t)(hi)) << 8) | (uint16_t)(lo)
#define COMBINE_UINT8_3(hi, mi, lo)                                                                \
    (((uint32_t)(hi)) << 16) | (((uint32_t)(mi)) << 8) | (uint32_t)(lo)

bool bmp_i2c_read(
    const bmp_inst* config, uint8_t* reg, size_t reg_len, uint8_t* dst, size_t dst_len)
{
    LOG_VERBOSE("BMP", "%d@%d->%d@%d", reg_len, *reg, dst_len, *dst);

    if (i2c_write_timeout_us(
            config->config.i2c, config->config.addr, reg, reg_len, true, BMP_TIMEOUT_US)
        < 1)
        return false;

    if (i2c_read_timeout_us(
            config->config.i2c, config->config.addr, dst, dst_len, false, BMP_TIMEOUT_US)
        < 1)
        return false;

    return true;
}

bool bmp_i2c_write(const bmp_inst* bmp, uint8_t* reg, uint8_t* data, size_t len)
{
    LOG_VERBOSE("BMP", "%d@%d", len, *reg);

    uint8_t buf[len * 2];

    for (uint8_t i = 0; i < len * 2; i++) {
        buf[i * 2 + 0] = reg[i];
        buf[i * 2 + 1] = data[i];
    }

    if (i2c_write_timeout_us(bmp->config.i2c, bmp->config.addr, buf, len * 2, false, BMP_TIMEOUT_US)
        < 1)
        return false;

    return true;
}

bool bmp_init(bmp_inst* bmp, bmp_config config)
{
    LOG_DEBUG("BMP", "");

    if (bmp == NULL)
        return false;

    bmp->config = config;

    i2c_init(bmp->config.i2c, 400 * 1000);

    gpio_set_function(bmp->config.sda, GPIO_FUNC_I2C);
    gpio_set_function(bmp->config.scl, GPIO_FUNC_I2C);
    gpio_pull_up(bmp->config.sda);
    gpio_pull_up(bmp->config.scl);

    if (bmp->read == NULL) {
        bmp->read = bmp_i2c_read;
    }
    if (bmp->write == NULL) {
        bmp->write = bmp_i2c_write;
    }

    bmp_pwr_config(bmp, false, false, BMP_MODE_SLEEP);

    bmp_read_configuration(bmp);

    return true;
}

void bmp_softreset(const bmp_inst* bmp)
{
    LOG_DEBUG("BMP", "");

    uint8_t value = BMP_CMD_SOFTRESET;

    uint8_t reg = BMP_REG_CMD;

    bmp->write(bmp, &reg, &value, 1);
}

bool bmp_pwr_config(bmp_inst* bmp, bool pressure_enable, bool temperature_enable, uint8_t mode)
{
    LOG_DEBUG("BMP", "Pressure enable: %d | Temperature enable: %d | Mode: %d", pressure_enable,
        temperature_enable, mode);

    bmp->sensor_config.enable_pressure    = pressure_enable;
    bmp->sensor_config.enable_temperature = temperature_enable;

    uint8_t value = ((uint8_t)pressure_enable) | (((uint8_t)temperature_enable) << 1) | (mode << 4);

    uint8_t reg = BMP_REG_PWR_CTRL;

    return bmp->write(bmp, &reg, &value, 1);
}

bool bmp_osr_config(const bmp_inst* bmp, uint8_t osr_p, uint8_t osr_t)
{
    LOG_DEBUG("BMP", "Pressure: %d | Temperature: %d", osr_p, osr_t);

    uint8_t value = osr_p | osr_t << 3;
    uint8_t reg   = BMP_REG_OSR;

    return bmp->write(bmp, &reg, &value, 1);
}

bool bmp_odr_config(const bmp_inst* bmp, uint8_t odr)
{
    LOG_DEBUG("BMP", "%d", odr);

    uint8_t value = odr;
    uint8_t reg   = BMP_REG_ODR;

    return bmp->write(bmp, &reg, &value, 1);
}

bool bmp_iir_config(const bmp_inst* bmp, uint8_t iir_filter)
{
    LOG_DEBUG("BMP", "%d", iir_filter);

    uint8_t value = iir_filter;
    uint8_t reg   = BMP_REG_CONFIG;

    return bmp->write(bmp, &reg, &value, 1);
}

void bmp_read_configuration(bmp_inst* bmp)
{
    LOG_DEBUG("BMP", "");

    uint8_t values[21];
    uint8_t reg = BMP_REG_NVM_T1L;

    bmp->read(bmp, &reg, 1, values, 21);

    bmp_calibration_int calibration;

    calibration.p11 = (int8_t)values[20];
    calibration.p10 = (int8_t)values[19];
    calibration.p9  = (int16_t)COMBINE_UINT8_2(values[18], values[17]);
    calibration.p8  = (int8_t)values[16];
    calibration.p7  = (int8_t)values[15];
    calibration.p6  = COMBINE_UINT8_2(values[14], values[13]);
    calibration.p5  = COMBINE_UINT8_2(values[12], values[11]);
    calibration.p4  = (int8_t)values[10];
    calibration.p3  = (int8_t)values[9];
    calibration.p2  = (int16_t)COMBINE_UINT8_2(values[8], values[7]);
    calibration.p1  = (int16_t)COMBINE_UINT8_2(values[6], values[5]);
    calibration.t3  = (int8_t)values[4];
    calibration.t2  = COMBINE_UINT8_2(values[3], values[2]);
    calibration.t1  = COMBINE_UINT8_2(values[1], values[0]);

    const double f2_m8   = 0.00390625;
    const double f2_m3   = 0.125;
    const double f2_6    = 64.0;
    const double f2_8    = 256.0;
    const double f2_14   = 16384.0;
    const double f2_15   = 32768.0;
    const double f2_20   = 1048576.0;
    const double f2_29   = 536870912.0;
    const double f2_30   = 1073741824.0;
    const double f2_32   = 4294967296.0;
    const double f2_37   = 137438953472.0;
    const double f2_48   = 281474976710656.0;
    const double f2_65   = 36893488147419103232.0;
    bmp->calibration.t1  = (double)calibration.t1 / (f2_m8);
    bmp->calibration.t2  = (double)calibration.t2 / (f2_30);
    bmp->calibration.t3  = (double)calibration.t3 / (f2_48);
    bmp->calibration.p1  = ((double)(calibration.p1 - f2_14)) / (f2_20);
    bmp->calibration.p2  = ((double)(calibration.p2 - f2_14)) / (f2_29);
    bmp->calibration.p3  = (double)calibration.p3 / (f2_32);
    bmp->calibration.p4  = (double)calibration.p4 / (f2_37);
    bmp->calibration.p5  = (double)calibration.p5 / (f2_m3);
    bmp->calibration.p6  = (double)calibration.p6 / (f2_6);
    bmp->calibration.p7  = (double)calibration.p7 / (f2_8);
    bmp->calibration.p8  = (double)calibration.p8 / (f2_15);
    bmp->calibration.p9  = (double)calibration.p9 / (f2_48);
    bmp->calibration.p10 = (double)calibration.p10 / (f2_48);
    bmp->calibration.p11 = (double)calibration.p11 / (f2_65);

    LOG_VERBOSE("BMP", "t1: %.15e", bmp->calibration.t1);
    LOG_VERBOSE("BMP", "t2: %.15e", bmp->calibration.t2);
    LOG_VERBOSE("BMP", "t3: %.15e", bmp->calibration.t3);
    LOG_VERBOSE("BMP", "p1: %.15e", bmp->calibration.p1);
    LOG_VERBOSE("BMP", "p2: %.15e", bmp->calibration.p2);
    LOG_VERBOSE("BMP", "p3: %.15e", bmp->calibration.p3);
    LOG_VERBOSE("BMP", "p4: %.15e", bmp->calibration.p4);
    LOG_VERBOSE("BMP", "p5: %.15e", bmp->calibration.p5);
    LOG_VERBOSE("BMP", "p6: %.15e", bmp->calibration.p6);
    LOG_VERBOSE("BMP", "p7: %.15e", bmp->calibration.p7);
    LOG_VERBOSE("BMP", "p8: %.15e", bmp->calibration.p8);
    LOG_VERBOSE("BMP", "p9: %.15e", bmp->calibration.p9);
    LOG_VERBOSE("BMP", "p10: %.15e", bmp->calibration.p10);
    LOG_VERBOSE("BMP", "p11: %.15e", bmp->calibration.p11);
}

uint8_t bmp_get_chip_id(const bmp_inst* bmp)
{
    LOG_DEBUG("BMP", "");

    uint8_t id;
    uint8_t target_address = BMP_REG_CHIP_ID;
    if (!bmp->read(bmp, &target_address, 1, &id, 1)) {
        return 0;
    }

    return id;
}

uint8_t bmp_get_error(const bmp_inst* bmp)
{
    LOG_DEBUG("BMP", "");

    uint8_t reg = BMP_REG_ERR;
    uint8_t val;
    bmp->read(bmp, &reg, 1, &val, 1);
    return val;
}

void bmp_read_values(bmp_inst* bmp)
{
    LOG_DEBUG("BMP", "");

    uint8_t temp[6];
    uint8_t addr = BMP_REG_PRESSURE_XLSB;

    if (!bmp->read(bmp, &addr, 1, temp, 6)) {
        return;
    }

    bmp->sensor_raw.press_raw = COMBINE_UINT8_3(temp[2], temp[1], temp[0]);
    bmp->sensor_raw.temp_raw  = COMBINE_UINT8_3(temp[5], temp[4], temp[3]);

    if (bmp->sensor_config.enable_temperature) {
        bmp_read_temperature(bmp);
    }

    if (bmp->sensor_config.enable_pressure) {
        bmp_read_pressure(bmp);
    }
}

void bmp_read_temperature(bmp_inst* bmp)
{
    LOG_DEBUG("BMP", "");

    double partial_data1, partial_data2;

    uint32_t temp_raw = bmp->sensor_raw.temp_raw;

    partial_data1 = (double)(temp_raw)-bmp->calibration.t1;
    partial_data2 = partial_data1 * bmp->calibration.t2;

    double temperature     = partial_data2 + (partial_data1 * partial_data1) * bmp->calibration.t3;
    bmp->calibration.t_lin = temperature;

    bmp->sensor_data.temperature = temperature;
}

void bmp_read_pressure(bmp_inst* bmp)
{
    LOG_DEBUG("BMP", "");

    double comp_press, partial_data1, partial_data2, partial_data3, partial_data4, partial_out1,
        partial_out2;

    uint32_t press_raw        = bmp->sensor_raw.press_raw;
    double   temperature      = bmp->calibration.t_lin;
    double   press_raw_double = press_raw;

    partial_data1 = bmp->calibration.p6 * temperature;
    partial_data2 = bmp->calibration.p7 * (temperature * temperature);
    partial_data3 = bmp->calibration.p8 * (temperature * temperature * temperature);
    partial_out1  = bmp->calibration.p5 + partial_data1 + partial_data2 + partial_data3;

    partial_data1 = bmp->calibration.p2 * temperature;
    partial_data2 = bmp->calibration.p3 * (temperature * temperature);
    partial_data3 = bmp->calibration.p4 * (temperature * temperature * temperature);
    partial_out2
        = press_raw_double * (bmp->calibration.p1 + partial_data1 + partial_data2 + partial_data3);

    partial_data1 = press_raw_double * press_raw_double;
    partial_data2 = bmp->calibration.p9 + bmp->calibration.p10 * temperature;
    partial_data3 = partial_data1 * partial_data2;
    partial_data4 = partial_data3
        + (press_raw_double * press_raw_double * press_raw_double) * bmp->calibration.p11;
    comp_press = partial_out1 + partial_out2 + partial_data4;

    bmp->sensor_data.pressure = comp_press;
}

float bmp_calc_altitude(float pressure, float sea_level)
{
    float pressure_hpa = pressure / 100.0f;
    return 44330.0f * (1.f - pow(pressure_hpa / sea_level, 0.1903f));
}
