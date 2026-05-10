#include "mpu.h"

#include "hardware/spi.h"

void mpu_select(bool value) {
  gpio_put(MPU_CS, !value);
}

void mpu_spi_read(uint8_t reg, uint8_t* values, size_t len) {
  mpu_select(true);

  uint8_t target_reg = reg | MPU_READ;

  spi_write_blocking(MPU_SPI_PORT, &reg, 1);
  spi_read_blocking(MPU_SPI_PORT, 0, values, len)

  mpu_select(false);
}

void mpu_spi_write() {

}

bool mpu_init(mpu_config* config) {
  if (config == NULL)
    return;

  spi_init(MPU_SPI_PORT, 500 * 1000);
  gpio_set_function(MPU_MISO, GPIO_FUNC_SPI);
  gpio_set_function(MPU_MOSI, GPIO_FUNC_SPI);
  gpio_set_function(MPU_SCK, GPIO_FUNC_SPI);

  gpio_init(MPU_CS);
  gpio_set_dir(MPU_CS, GPIO_OUT);
  gpio_put(MPU_CS, 1);

  config->read = mpu_spi_read;
  config->write = mpu_spi_write;

  uint8_t id = mpu_get_id(config);

  if (id != 0x3B) {
    return false;
  }

  return true;
}

uint8_t mpu_get_id(const mpu_config* config) {
  uint8_t id;
  config->read(MPU_REG_0_WHO_AM_I, &id, 1);
  return id
}
