#pragma once

#define BMP_I2C_PORT i2c1
#define BMP_ADDR     0x76
#define BMP_SDA      6
#define BMP_SCL      7

#define ICM_SPI_PORT spi0
#define ICM_MISO     4
#define ICM_MOSI     3
#define ICM_CS       5
#define ICM_SCK      2

#define LORA_SPI_PORT spi1
#define LORA_MISO     12
#define LORA_MOSI     11
#define LORA_CS       9
#define LORA_SCK      10
#define LORA_BUSY     13
#define LORA_RST      8
#define LORA_DIO1     14

#define CONFIG_LORA_SF              LORA_SF_12
#define CONFIG_LORA_BW              LORA_BW_250
#define CONFIG_LORA_CR              LORA_CR_4_5
#define CONFIG_LORA_PREAMBLE        0x08
#define CONFIG_LORA_IMPLICIT_HEADER false
#define CONFIG_LORA_CRC             true
#define CONFIG_LORA_INVERT_IQ       false
#define CONFIG_LORA_POWER           14
#define CONFIG_LORA_RAMP_TIME       LORA_RAMP_TIME_200U

#define CONFIG_BMP_OSR_P BMP_OVERSAMPLE_X2
#define CONFIG_BMP_OSR_T BMP_OVERSAMPLE_X1
#define CONFIG_BMP_ODR   BMP_ODR_100
#define CONFIG_BMP_IIR   BMP_IIR_COEF_0

#define CONFIG_ICM_ACCEL_MODE ICM_PWR_LOW_NOISE
#define CONFIG_ICM_GYRO_MODE  ICM_PWR_LOW_NOISE
#define CONFIG_ICM_ACCEL_FSR  ICM_ACCEL_FSR_32g
#define CONFIG_ICM_ACCEL_ODR  ICM_ACCEL_ODR_2_KHZ
#define CONFIG_ICM_GYRO_FSR   ICM_GYRO_FSR_500
#define CONFIG_ICM_GYRO_ODR   ICM_GYRO_ODR_2_KHZ
