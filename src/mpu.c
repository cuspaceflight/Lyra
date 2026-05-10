#include "mpu.h"

#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <string.h>

#include "pico/stdlib.h"
#include "util.h"

void mpu_spi_select(const mpu_config* config, bool value) { gpio_put(config->cs, !value); }

void mpu_spi_write(const mpu_config* config, uint8_t reg, uint8_t* data, size_t len)
{
    uint8_t tx[1 + len];
    uint8_t rx[1 + len];

    tx[0] = reg & ~MPU_READ;
    memcpy(&tx[1], data, len);

    config->select(config, true);
    spi_write_read_blocking(config->spi, tx, rx, len + 1);
    config->select(config, false);
}

void mpu_spi_read(const mpu_config* config, uint8_t reg, uint8_t* values, size_t len)
{
    uint8_t tx[1 + len];
    uint8_t rx[1 + len];

    tx[0] = reg | MPU_READ;
    memset(&tx[1], 0, len);

    config->select(config, true);
    spi_write_read_blocking(config->spi, tx, rx, len + 1);
    config->select(config, false);

    memcpy(values, &rx[1], len);
}

void mpu_defaults(mpu_config* config)
{
    config->mosi = MPU_MOSI;
    config->miso = MPU_MISO;
    config->sck  = MPU_SCK;
    config->cs   = MPU_CS;
    config->spi  = MPU_SPI_PORT;

    config->select = mpu_spi_select;
    config->write  = mpu_spi_write;
    config->read   = mpu_spi_read;
}

bool mpu_init(mpu_config* config)
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

    if (mpu_get_id(config) != 0x3B) {
        return false;
    }

    return true;
}

void mpu_set_bank(const mpu_config* config, uint8_t bank)
{
    config->write(config, MPU_REG_BANK_SEL, &bank, 1);
}

void mpu_pwr_mgmt(
    const mpu_config* config, uint8_t accel_mode, uint8_t gyro_mode, bool idle, bool disable_temp)
{
    uint8_t value = (accel_mode & 0b11) | ((gyro_mode & 0b11) << 2) | ((uint8_t)(!idle) << 4)
        | ((uint8_t)disable_temp << 5);

    mpu_set_bank(config, MPU_BANK_0);

    config->write(config, MPU_REG_0_PWR_MGMT0, &value, 1);
}

uint8_t mpu_get_id(const mpu_config* config)
{
    mpu_set_bank(config, MPU_BANK_0);

    uint8_t id;
    config->read(config, MPU_REG_0_WHO_AM_I, &id, 1);
    return id;
}

float convert_raw_temperature(uint16_t temperature) { return ((float)temperature / 132.48f) + 25; }

void mpu_read_data(mpu_config* config)
{
    uint8_t values[14];
    config->read(config, MPU_REG_0_TEMP_DATA1, values, 14);

    config->sensor_data.temperature
        = convert_raw_temperature(COMBINE_UINT8_2(values[0], values[1]));
    config->sensor_data.accel[0] = COMBINE_UINT8_2(values[2], values[3]);
    config->sensor_data.accel[1] = COMBINE_UINT8_2(values[4], values[5]);
    config->sensor_data.accel[2] = COMBINE_UINT8_2(values[6], values[7]);

    config->sensor_data.gyro[0] = COMBINE_UINT8_2(values[8], values[9]);
    config->sensor_data.gyro[1] = COMBINE_UINT8_2(values[10], values[11]);
    config->sensor_data.gyro[2] = COMBINE_UINT8_2(values[12], values[13]);
}

float mpu_read_temperature(const mpu_config* config)
{
    uint8_t values[2];
    config->read(config, MPU_REG_0_TEMP_DATA1, values, 2);

    int16_t temperature = (int16_t)COMBINE_UINT8_2(values[0], values[1]);
    return ((float)temperature / 132.48f) + 25;
}
