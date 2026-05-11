#include "icm40609D.h"

#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <string.h>

#include "pico/stdlib.h"
#include "util.h"

void icm_spi_select(const icm_config* config, bool value) { gpio_put(config->cs, !value); }

void icm_spi_write(const icm_config* config, uint8_t reg, uint8_t* data, size_t len)
{
    uint8_t tx[1 + len];
    uint8_t rx[1 + len];

    tx[0] = reg & ~ICM_READ;
    memcpy(&tx[1], data, len);

    config->select(config, true);
    spi_write_read_blocking(config->spi, tx, rx, len + 1);
    config->select(config, false);
}

void icm_spi_read(const icm_config* config, uint8_t reg, uint8_t* values, size_t len)
{
    uint8_t tx[1 + len];
    uint8_t rx[1 + len];

    tx[0] = reg | ICM_READ;
    memset(&tx[1], 0, len);

    config->select(config, true);
    spi_write_read_blocking(config->spi, tx, rx, len + 1);
    config->select(config, false);

    memcpy(values, &rx[1], len);
}

void icm_defaults(icm_config* config)
{
    config->mosi = ICM_MOSI;
    config->miso = ICM_MISO;
    config->sck  = ICM_SCK;
    config->cs   = ICM_CS;
    config->spi  = ICM_SPI_PORT;

    config->select = icm_spi_select;
    config->write  = icm_spi_write;
    config->read   = icm_spi_read;
}

bool icm_init(icm_config* config)
{
    if (config == NULL)
        return false;

    spi_init(config->spi, 500 * 1000);
    gpio_set_function(config->miso, GPIO_FUNC_SPI);
    gpio_set_function(config->mosi, GPIO_FUNC_SPI);
    gpio_set_function(config->sck, GPIO_FUNC_SPI);

    gpio_init(config->cs);
    gpio_set_dir(config->cs, GPIO_OUT);
    gpio_put(config->cs, 1);

    gpio_pull_up(config->miso);

    spi_set_format(config->spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    if (icm_get_id(config) != 0x3B) {
        return false;
    }

    return true;
}

void icm_set_bank(const icm_config* config, uint8_t bank)
{
    config->write(config, ICM_REG_BANK_SEL, &bank, 1);
}

void icm_pwr_mgmt(
    const icm_config* config, uint8_t accel_mode, uint8_t gyro_mode, bool idle, bool disable_temp)
{
    uint8_t value = (accel_mode & 0b11) | ((gyro_mode & 0b11) << 2) | ((uint8_t)(!idle) << 4)
        | ((uint8_t)disable_temp << 5);

    icm_set_bank(config, ICM_BANK_0);

    config->write(config, ICM_REG_0_PWR_MGMT0, &value, 1);
}

void icm_gyro_config(icm_config* config, uint8_t fsr, uint8_t odr)
{
    uint8_t value = (fsr << 5) | odr;

    icm_set_bank(config, ICM_BANK_0);
    config->write(config, ICM_REG_0_GYRO_CONFIG0, &value, 1);

    switch (fsr) {
    case ICM_GYRO_FSR_2000:
        config->sensor_correction.gyro_ssf = 16.4f;
        break;
    case ICM_GYRO_FSR_1000:
        config->sensor_correction.gyro_ssf = 32.8f;
        break;
    case ICM_GYRO_FSR_500:
        config->sensor_correction.gyro_ssf = 65.5f;
        break;
    case ICM_GYRO_FSR_250:
        config->sensor_correction.gyro_ssf = 131.f;
        break;
    case ICM_GYRO_FSR_125:
        config->sensor_correction.gyro_ssf = 262.f;
        break;
    case ICM_GYRO_FSR_62p5:
        config->sensor_correction.gyro_ssf = 524.3f;
        break;
    case ICM_GYRO_FSR_31p25:
        config->sensor_correction.gyro_ssf = 1048.6f;
        break;
    case ICM_GYRO_FSR_15p625:
        config->sensor_correction.gyro_ssf = 2097.2f;
        break;
    }
}

void icm_accel_config(icm_config* config, uint8_t fsr, uint8_t odr)
{
    uint8_t value = (fsr << 5) | odr;

    icm_set_bank(config, ICM_BANK_0);
    config->write(config, ICM_REG_0_ACCEL_CONFIG0, &value, 1);

    switch (fsr) {
    case ICM_ACCEL_FSR_32g:
        config->sensor_correction.accel_ssf = 1024.f;
        break;
    case ICM_ACCEL_FSR_16g:
        config->sensor_correction.accel_ssf = 2048.f;
        break;
    case ICM_ACCEL_FSR_8g:
        config->sensor_correction.accel_ssf = 4096.f;
        break;
    case ICM_ACCEL_FSR_4g:
        config->sensor_correction.accel_ssf = 8192.f;
        break;
    }
}

uint8_t icm_get_id(const icm_config* config)
{
    icm_set_bank(config, ICM_BANK_0);

    uint8_t id;
    config->read(config, ICM_REG_0_WHO_AM_I, &id, 1);
    return id;
}

float convert_raw_temperature(int16_t temperature) { return ((float)temperature / 132.48f) + 25; }

float convert_raw_accel(const icm_config* config, int16_t accel)
{
    return ((float)accel / config->sensor_correction.accel_ssf) * 9.80665f;
}

float convert_raw_gyro(const icm_config* config, int16_t gyro)
{
    return (float)gyro / config->sensor_correction.gyro_ssf;
}

void icm_read_data(icm_config* config)
{
    uint8_t values[14];
    config->read(config, ICM_REG_0_TEMP_DATA1, values, 14);

    config->sensor_data.temperature
        = convert_raw_temperature(COMBINE_UINT8_2(values[0], values[1]));
    config->sensor_data.accel[0] = convert_raw_accel(config, COMBINE_UINT8_2(values[2], values[3]));
    config->sensor_data.accel[1] = convert_raw_accel(config, COMBINE_UINT8_2(values[4], values[5]));
    config->sensor_data.accel[2] = convert_raw_accel(config, COMBINE_UINT8_2(values[6], values[7]));

    config->sensor_data.gyro[0] = convert_raw_gyro(config, COMBINE_UINT8_2(values[8], values[9]));
    config->sensor_data.gyro[1] = convert_raw_gyro(config, COMBINE_UINT8_2(values[10], values[11]));
    config->sensor_data.gyro[2] = convert_raw_gyro(config, COMBINE_UINT8_2(values[12], values[13]));
}

float icm_read_temperature(const icm_config* config)
{
    uint8_t values[2];
    config->read(config, ICM_REG_0_TEMP_DATA1, values, 2);

    int16_t temperature = (int16_t)COMBINE_UINT8_2(values[0], values[1]);
    return ((float)temperature / 132.48f) + 25;
}
