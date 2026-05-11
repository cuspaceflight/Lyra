#include "bmp388.h"

#include "bmp388_defines.h"

#include "pico/stdlib.h"
#include <math.h>

#define COMBINE_UINT8_2(hi, lo) (((uint16_t)(hi)) << 8) | (uint16_t)(lo)
#define COMBINE_UINT8_3(hi, mi, lo)                                                                \
    (((uint32_t)(hi)) << 16) | (((uint32_t)(mi)) << 8) | (uint32_t)(lo)

bool bmp_i2c_read(
    const bmp_inst* config, uint8_t* reg, size_t reg_len, uint8_t* dst, size_t dst_len)
{
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

    bmp_read_configuration(bmp);

    return true;
}

bool bmp_pwr_config(
    const bmp_inst* bmp, bool pressure_enable, bool temperature_enable, uint8_t mode)
{
    uint8_t value = ((uint8_t)pressure_enable) | (((uint8_t)temperature_enable) << 1) | (mode << 4);

    uint8_t reg = BMP_REG_PWR_CTRL;

    return bmp->write(bmp, &reg, &value, 1);
}

bool bmp_osr_config(const bmp_inst* bmp, uint8_t osr_p, uint8_t osr_t)
{
    uint8_t value = osr_p | osr_t << 3;
    uint8_t reg   = BMP_REG_OSR;

    return bmp->write(bmp, &reg, &value, 1);
}

bool bmp_odr_config(const bmp_inst* bmp, uint8_t odr)
{
    uint8_t value = odr;
    uint8_t reg   = BMP_REG_ODR;

    return bmp->write(bmp, &reg, &value, 1);
}

bool bmp_iir_config(const bmp_inst* bmp, uint8_t iir_filter)
{
    uint8_t value = iir_filter;
    uint8_t reg   = BMP_REG_CONFIG;

    return bmp->write(bmp, &reg, &value, 1);
}

void bmp_read_configuration(bmp_inst* bmp)
{
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

    const float f2_m8    = 0.00390625f;
    const float f2_m3    = 0.125f;
    const float f2_6     = 64.f;
    const float f2_8     = 256.f;
    const float f2_14    = 16384.f;
    const float f2_15    = 32768.f;
    const float f2_20    = 1048576.f;
    const float f2_29    = 536870912.f;
    const float f2_30    = 1073741824.f;
    const float f2_32    = 4294967296.f;
    const float f2_37    = 137438953472.f;
    const float f2_48    = 281474976710656.f;
    const float f2_65    = 36893488147419103232.f;
    bmp->calibration.t1  = (float)calibration.t1 / (f2_m8);
    bmp->calibration.t2  = (float)calibration.t2 / (f2_30);
    bmp->calibration.t3  = (float)calibration.t3 / (f2_48);
    bmp->calibration.p1  = ((float)calibration.p1 - f2_14) / (f2_20);
    bmp->calibration.p2  = ((float)calibration.p2 - f2_14) / (f2_29);
    bmp->calibration.p3  = (float)calibration.p3 / (f2_32);
    bmp->calibration.p4  = (float)calibration.p4 / (f2_37);
    bmp->calibration.p5  = (float)calibration.p5 / (f2_m3);
    bmp->calibration.p6  = (float)calibration.p6 / (f2_6);
    bmp->calibration.p7  = (float)calibration.p7 / (f2_8);
    bmp->calibration.p8  = (float)calibration.p8 / (f2_15);
    bmp->calibration.p9  = (float)calibration.p9 / (f2_48);
    bmp->calibration.p10 = (float)calibration.p10 / (f2_48);
    bmp->calibration.p11 = (float)calibration.p11 / (f2_65);
}

uint8_t bmp_get_chip_id(const bmp_inst* bmp)
{
    uint8_t id;
    uint8_t target_address = BMP_REG_CHIP_ID;
    if (!bmp->read(bmp, &target_address, 1, &id, 1)) {
        return 0;
    }

    return id;
}

uint32_t bmp_read_temperature_raw(const bmp_inst* bmp)
{
    uint8_t temp[3];
    uint8_t addr = BMP_REG_TEMPERATURE_XLSB;

    if (!bmp->read(bmp, &addr, 1, temp, 3)) {
        return 1;
    }

    return COMBINE_UINT8_3(temp[2], temp[1], temp[0]);
}

float bmp_read_temperature(const bmp_inst* bmp)
{
    float partial_data1, partial_data2;

    uint32_t temp_raw = bmp_read_temperature_raw(bmp);

    partial_data1 = (float)(temp_raw - bmp->calibration.t1);
    partial_data2 = (float)(partial_data1 * bmp->calibration.t2);

    float temperature = partial_data2 + (partial_data1 * partial_data1) * bmp->calibration.t3;
    return temperature;
}

uint32_t bmp_read_pressure_raw(const bmp_inst* bmp)
{
    uint8_t press[3];
    uint8_t addr = BMP_REG_PRESSURE_XLSB;

    if (!bmp->read(bmp, &addr, 1, press, 3)) {
        return 1;
    }

    return COMBINE_UINT8_3(press[2], press[1], press[0]);
}

float bmp_read_pressure(const bmp_inst* bmp, float temperature)
{
    float comp_press, partial_data1, partial_data2, partial_data3, partial_data4, partial_out1,
        partial_out2;

    uint32_t press_raw = bmp_read_pressure_raw(bmp);

    partial_data1 = bmp->calibration.p6 * temperature;
    partial_data2 = bmp->calibration.p7 * (temperature * temperature);
    partial_data3 = bmp->calibration.p8 * (temperature * temperature * temperature);
    partial_out1  = bmp->calibration.p5 + partial_data1 + partial_data2 + partial_data3;

    partial_data1 = bmp->calibration.p2 * temperature;
    partial_data2 = bmp->calibration.p3 * (temperature * temperature);
    partial_data3 = bmp->calibration.p4 * (temperature * temperature * temperature);
    partial_out2
        = (float)press_raw * (bmp->calibration.p1 + partial_data1 + partial_data2 + partial_data3);

    partial_data1 = (float)press_raw * (float)press_raw;
    partial_data2 = bmp->calibration.p9 + bmp->calibration.p10 * temperature;
    partial_data3 = partial_data1 * partial_data2;
    partial_data4 = partial_data3
        + ((float)press_raw * (float)press_raw * (float)press_raw) * bmp->calibration.p11;
    comp_press = partial_out1 + partial_out2 + partial_data4;

    return comp_press;
}

float bmp_calc_altitude(float pressure, float sea_level)
{
    float pressure_hpa = pressure / 100.0f;
    return 44330.0f * (1.f - pow(pressure_hpa / sea_level, 0.1903f));
}
