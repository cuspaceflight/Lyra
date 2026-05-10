#pragma once

#include "hardware/spi.h"

#define MPU_SPI_PORT spi0
#define MPU_MISO     4
#define MPU_MOSI     3
#define MPU_CS       5
#define MPU_SCK      2

#define MPU_REG_BANK_SEL 0x76
#define MPU_BANK_0       0b000
#define MPU_BANK_1       0b001
#define MPU_BANK_2       0b010
#define MPU_BANK_3       0b011
#define MPU_BANK_4       0b100

#define MPU_REG_0_DEVICE_CONFIG      0x11
#define MPU_REG_0_DRIVE_CONFIG       0x13
#define MPU_REG_0_INT_CONFIG         0x14
#define MPU_REG_0_FIFO_CONFIG        0x16
#define MPU_REG_0_TEMP_DATA1         0x1D
#define MPU_REG_0_TEMP_DATA0         0x1E
#define MPU_REG_0_ACCEL_DATA_X1      0x1F
#define MPU_REG_0_ACCEL_DATA_X0      0x20
#define MPU_REG_0_ACCEL_DATA_Y1      0x21
#define MPU_REG_0_ACCEL_DATA_Y0      0x22
#define MPU_REG_0_ACCEL_DATA_Z1      0x23
#define MPU_REG_0_ACCEL_DATA_Z0      0x24
#define MPU_REG_0_GYRO_DATA_X1       0x25
#define MPU_REG_0_GYRO_DATA_X0       0x26
#define MPU_REG_0_GYRO_DATA_Y1       0x27
#define MPU_REG_0_GYRO_DATA_Y0       0x28
#define MPU_REG_0_GYRO_DATA_Z1       0x29
#define MPU_REG_0_GYRO_DATA_Z0       0x2A
#define MPU_REG_0_TMST_FSYNCH        0x2B
#define MPU_REG_0_TMST_FSYNCL        0x2C
#define MPU_REG_0_INT_STATUS         0x2D
#define MPU_REG_0_FIFO_COUNTH        0x2E
#define MPU_REG_0_FIFO_COUNTL        0x2F
#define MPU_REG_0_FIFO_DATA          0x30
#define MPU_REG_0_INT_STATUS2        0x37
#define MPU_REG_0_SIGNAL_PATH_RESET  0x4B
#define MPU_REG_0_INTF_CONFIG0       0x4C
#define MPU_REG_0_INTF_CONFIG1       0x4D
#define MPU_REG_0_PWR_MGMT0          0x4E
#define MPU_REG_0_GYRO_CONFIG0       0x4F
#define MPU_REG_0_ACCEL_CONFIG0      0x50
#define MPU_REG_0_GYRO_CONFIG1       0x51
#define MPU_REG_0_GYRO_ACCEL_CONFIG0 0x52
#define MPU_REG_0_ACCEL_CONFIG1      0x53
#define MPU_REG_0_TMST_CONFIG        0x54
#define MPU_REG_0_TMST_CONFIG        0x54
#define MPU_REG_0_FIFO_CONFIG1       0x5F
#define MPU_REG_0_FIFO_CONFIG2       0x60
#define MPU_REG_0_FIFO_CONFIG3       0x61
#define MPU_REG_0_FSYNC_CONFIG       0x62
#define MPU_REG_0_INT_CONFIG0        0x63
#define MPU_REG_0_INT_CONFIG1        0x64
#define MPU_REG_0_INT_SOURCE0        0x65
#define MPU_REG_0_INT_SOURCE1        0x66
#define MPU_REG_0_INT_SOURCE3        0x68
#define MPU_REG_0_INT_SOURCE4        0x69
#define MPU_REG_0_FIFO_LOST_PKT0     0x6C
#define MPU_REG_0_FIFO_LOST_PKT1     0x6D
#define MPU_REG_0_SELF_TEST_CONFIG   0x70
#define MPU_REG_0_WHO_AM_I           0x75

#define MPU_REG_1_SENSOR_CONFIG0       0x03
#define MPU_REG_1_GYRO_CONFIG_STATIC2  0x0B
#define MPU_REG_1_GYRO_CONFIG_STATIC3  0x0C
#define MPU_REG_1_GYRO_CONFIG_STATIC4  0x0D
#define MPU_REG_1_GYRO_CONFIG_STATIC5  0x0E
#define MPU_REG_1_GYRO_CONFIG_STATIC7  0x10
#define MPU_REG_1_GYRO_CONFIG_STATIC8  0x11
#define MPU_REG_1_GYRO_CONFIG_STATIC9  0x12
#define MPU_REG_1_GYRO_CONFIG_STATIC10 0x13
#define MPU_REG_1_XG_ST_DATA           0x5F
#define MPU_REG_1_YG_ST_DATA           0x60
#define MPU_REG_1_ZG_ST_DATA           0x61
#define MPU_REG_1_TMSTVAL0             0x52
#define MPU_REG_1_TMSTVAL1             0x63
#define MPU_REG_1_TMSTVAL2             0x64
#define MPU_REG_1_INTF_CONFIG4         0x7A
#define MPU_REG_1_INTF_CONFIG5         0x7B

#define MPU_REG_2_ACCEL_CONFIG_STATIC2 0x03
#define MPU_REG_2_ACCEL_CONFIG_STATIC3 0x04
#define MPU_REG_2_ACCEL_CONFIG_STATIC4 0x05
#define MPU_REG_2_XA_ST_DATA           0x3B
#define MPU_REG_2_YA_ST_DATA           0x3C
#define MPU_REG_2_ZA_ST_DATA           0x3D

#define MPU_REG_3_ACCEL_WOM_X_THR 0x4A
#define MPU_REG_3_ACCEL_WOM_Y_THR 0x4B
#define MPU_REG_3_ACCEL_WOM_Z_THR 0x4C
#define MPU_REG_3_OFFSET_USER0    0x77
#define MPU_REG_3_OFFSET_USER1    0x78
#define MPU_REG_3_OFFSET_USER2    0x79
#define MPU_REG_3_OFFSET_USER3    0x7A
#define MPU_REG_3_OFFSET_USER4    0x7B
#define MPU_REG_3_OFFSET_USER5    0x7C
#define MPU_REG_3_OFFSET_USER6    0x7D
#define MPU_REG_3_OFFSET_USER7    0x7E
#define MPU_REG_3_OFFSET_USER8    0x7F

#define MPU_READ 0x80

#define MPU_PWR_OFF       0b00
#define MPU_PWR_STANDBY   0b01
#define MPU_PWR_LOW_POWER 0b10
#define MPU_PWR_LOW_NOISE 0b11

typedef struct mpu_sensor_data_t {
    float    temperature;
    uint16_t accel[3];
    uint16_t gyro[3];
} mpu_sensor_data;

typedef struct mpu_config_t {
    spi_inst_t* spi;
    uint8_t     miso;
    uint8_t     mosi;
    uint8_t     sck;
    uint8_t     cs;

    mpu_sensor_data sensor_data;

    void (*select)(const struct mpu_config_t*, bool);
    void (*write)(const struct mpu_config_t*, uint8_t, uint8_t*, size_t);
    void (*read)(const struct mpu_config_t*, uint8_t, uint8_t*, size_t);
} mpu_config;

void mpu_defaults(mpu_config* config);
bool mpu_init(mpu_config* config);

void mpu_set_bank(const mpu_config* config, uint8_t bank);

void mpu_pwr_mgmt(
    const mpu_config* config, uint8_t accel_mode, uint8_t gyro_mode, bool idle, bool disable_temp);

uint8_t mpu_get_id(const mpu_config* config);

void  mpu_read_data(mpu_config* config);
float mpu_read_temperature(const mpu_config* config);
