
#include "icm40609D.h"

#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <string.h>

#include "logger/logger.h"

#include "pico/stdlib.h"

#define COMBINE_UINT8_2(hi, lo) ((uint16_t)(hi) << 8) | (uint16_t)(lo)

void icm_spi_select(const icm_inst* icm, bool value)
{
    LOG_VERBOSE("ICM", "%d", !value);
    gpio_put(icm->config.cs, !value);
}

void icm_spi_write(const icm_inst* icm, uint8_t reg, uint8_t* data, size_t len)
{
    LOG_VERBOSE("ICM", "%d<-", reg);

    uint8_t tx[1 + len];
    uint8_t rx[1 + len];

    tx[0] = reg & ~ICM_READ;
    memcpy(&tx[1], data, len);

    icm->select(icm, true);
    spi_write_read_blocking(icm->config.spi, tx, rx, len + 1);
    icm->select(icm, false);
}

void icm_spi_read(const icm_inst* icm, uint8_t reg, uint8_t* values, size_t len)
{
    LOG_VERBOSE("ICM", "%d->", reg);

    uint8_t tx[1 + len];
    uint8_t rx[1 + len];

    tx[0] = reg | ICM_READ;
    memset(&tx[1], 0, len);

    icm->select(icm, true);
    spi_write_read_blocking(icm->config.spi, tx, rx, len + 1);
    icm->select(icm, false);

    memcpy(values, &rx[1], len);
}

bool icm_init(icm_inst* icm, icm_config config)
{
    LOG_DEBUG("ICM", "");

    if (icm == NULL)
        return false;

    icm->config = config;

    spi_init(icm->config.spi, 500 * 1000);
    gpio_set_function(icm->config.miso, GPIO_FUNC_SPI);
    gpio_set_function(icm->config.mosi, GPIO_FUNC_SPI);
    gpio_set_function(icm->config.sck, GPIO_FUNC_SPI);

    gpio_init(icm->config.cs);
    gpio_set_dir(icm->config.cs, GPIO_OUT);
    gpio_put(icm->config.cs, 1);

    gpio_pull_up(icm->config.miso);

    spi_set_format(icm->config.spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    if (icm->select == NULL) {
        icm->select = icm_spi_select;
    }
    if (icm->write == NULL) {
        icm->write = icm_spi_write;
    }
    if (icm->read == NULL) {
        icm->read = icm_spi_read;
    }

    if (icm_get_id(icm) != 0x3B) {
        return false;
    }

    return true;
}

void icm_set_bank(const icm_inst* icm, uint8_t bank)
{
    LOG_DEBUG("ICM", "%d", bank);
    icm->write(icm, ICM_REG_BANK_SEL, &bank, 1);
}

void icm_pwr_mgmt(
    const icm_inst* icm, uint8_t accel_mode, uint8_t gyro_mode, bool idle, bool disable_temp)
{
    LOG_DEBUG("ICM", "Accel Mode: %d | Gyro Mode: %d | Idle: %d | Enable Temp: %d", accel_mode,
        gyro_mode, idle, disable_temp);

    uint8_t value = (accel_mode & 0b11) | ((gyro_mode & 0b11) << 2) | ((uint8_t)(!idle) << 4)
        | ((uint8_t)disable_temp << 5);

    icm_set_bank(icm, ICM_BANK_0);

    icm->write(icm, ICM_REG_0_PWR_MGMT0, &value, 1);
}

void icm_gyro_config(icm_inst* icm, uint8_t fsr, uint8_t odr)
{
    LOG_DEBUG("ICM", "FSR: %d | ODR: %d", fsr, odr);

    uint8_t value = (fsr << 5) | odr;

    icm_set_bank(icm, ICM_BANK_0);
    icm->write(icm, ICM_REG_0_GYRO_CONFIG0, &value, 1);

    switch (fsr) {
    case ICM_GYRO_FSR_2000:
        icm->sensor_correction.gyro_ssf = 16.4f;
        break;
    case ICM_GYRO_FSR_1000:
        icm->sensor_correction.gyro_ssf = 32.8f;
        break;
    case ICM_GYRO_FSR_500:
        icm->sensor_correction.gyro_ssf = 65.5f;
        break;
    case ICM_GYRO_FSR_250:
        icm->sensor_correction.gyro_ssf = 131.f;
        break;
    case ICM_GYRO_FSR_125:
        icm->sensor_correction.gyro_ssf = 262.f;
        break;
    case ICM_GYRO_FSR_62p5:
        icm->sensor_correction.gyro_ssf = 524.3f;
        break;
    case ICM_GYRO_FSR_31p25:
        icm->sensor_correction.gyro_ssf = 1048.6f;
        break;
    case ICM_GYRO_FSR_15p625:
        icm->sensor_correction.gyro_ssf = 2097.2f;
        break;
    }
}

void icm_accel_config(icm_inst* icm, uint8_t fsr, uint8_t odr)
{
    LOG_DEBUG("ICM", "FSR: %d | ODR: %d", fsr, odr);

    uint8_t value = (fsr << 5) | odr;

    icm_set_bank(icm, ICM_BANK_0);
    icm->write(icm, ICM_REG_0_ACCEL_CONFIG0, &value, 1);

    switch (fsr) {
    case ICM_ACCEL_FSR_32g:
        icm->sensor_correction.accel_ssf = 1024.f;
        break;
    case ICM_ACCEL_FSR_16g:
        icm->sensor_correction.accel_ssf = 2048.f;
        break;
    case ICM_ACCEL_FSR_8g:
        icm->sensor_correction.accel_ssf = 4096.f;
        break;
    case ICM_ACCEL_FSR_4g:
        icm->sensor_correction.accel_ssf = 8192.f;
        break;
    }
}

uint8_t icm_get_id(const icm_inst* icm)
{
    LOG_DEBUG("ICM", "");

    icm_set_bank(icm, ICM_BANK_0);

    uint8_t id;
    icm->read(icm, ICM_REG_0_WHO_AM_I, &id, 1);
    return id;
}

float convert_raw_temperature(int16_t temperature)
{
    LOG_DEBUG("ICM", "%d", temperature);

    return ((float)temperature / 132.48f) + 25;
}

float convert_raw_accel(const icm_inst* icm, int16_t accel)
{
    LOG_DEBUG("ICM", "%d", accel);

    return ((float)accel / icm->sensor_correction.accel_ssf) * 9.80665f;
}

float convert_raw_gyro(const icm_inst* icm, int16_t gyro)
{
    LOG_DEBUG("ICM", "%d", gyro);

    return (float)gyro / icm->sensor_correction.gyro_ssf;
}

void icm_read_data(icm_inst* icm)
{
    LOG_DEBUG("ICM", "");

    uint8_t values[14];
    icm->read(icm, ICM_REG_0_TEMP_DATA1, values, 14);

    icm->sensor_data.temperature = convert_raw_temperature(COMBINE_UINT8_2(values[0], values[1]));
    icm->sensor_data.accel[0]    = convert_raw_accel(icm, COMBINE_UINT8_2(values[2], values[3]));
    icm->sensor_data.accel[1]    = convert_raw_accel(icm, COMBINE_UINT8_2(values[4], values[5]));
    icm->sensor_data.accel[2]    = convert_raw_accel(icm, COMBINE_UINT8_2(values[6], values[7]));

    icm->sensor_data.gyro[0] = convert_raw_gyro(icm, COMBINE_UINT8_2(values[8], values[9]));
    icm->sensor_data.gyro[1] = convert_raw_gyro(icm, COMBINE_UINT8_2(values[10], values[11]));
    icm->sensor_data.gyro[2] = convert_raw_gyro(icm, COMBINE_UINT8_2(values[12], values[13]));
}

float icm_read_temperature(const icm_inst* icm)
{
    LOG_DEBUG("ICM", "");

    uint8_t values[2];
    icm->read(icm, ICM_REG_0_TEMP_DATA1, values, 2);

    int16_t temperature = (int16_t)COMBINE_UINT8_2(values[0], values[1]);
    return ((float)temperature / 132.48f) + 25;
}
