#pragma once

#define LORA_CMD_SET_SLEEP                0x84
#define LORA_CMD_SET_STANDBY              0x80
#define LORA_CMD_SET_FS                   0xC1
#define LORA_CMD_SET_TX                   0x83
#define LORA_CMD_SET_RX                   0x82
#define LORA_CMD_STOP_TIMER_ON_PREAMBLE   0x9F
#define LORA_CMD_SET_RX_DUTY_CYCLE        0x94
#define LORA_CMD_SET_CAD                  0xC5
#define LORA_CMD_SET_TX_CONTINUOUS_WAVE   0xD1
#define LORA_CMD_SET_TX_INFINITE_PREAMBLE 0xD2
#define LORA_CMD_SET_REGULATOR_MODE       0x96
#define LORA_CMD_CALIBRATE                0x89
#define LORA_CMD_CALIBRATE_IMAGE          0x98
#define LORA_CMD_SET_PA_CONFIG            0x95
#define LORA_CMD_SET_RX_TX_FALLBACK_CYCLE 0x93

#define LORA_MEM_WRITE_REGISTER 0x0D
#define LORA_MEM_READ_REGISTER  0x1D
#define LORA_MEM_WRITE_BUFFER   0x0E
#define LORA_MEM_READ_BUFFER    0x1E

#define LORA_CMD_SET_DIO_IRQ_PARAMS         0x08
#define LORA_CMD_GET_IRQ_STATUS             0x12
#define LORA_CMD_CLEAR_IRQ_STATUS           0x02
#define LORA_CMD_SET_DIO2_AS_RF_SWITCH_CTRL 0x9D
#define LORA_CMD_SET_DIO3_AS_TCXO_CTRL      0x97

#define LORA_CMD_SET_RF_FREQUENCY          0x86
#define LORA_CMD_SET_PACKET_TYPE           0x8A
#define LORA_CMD_GET_PACKET_TYPE           0x11
#define LORA_CMD_SET_TX_PARAMS             0x8E
#define LORA_CMD_SET_MODULATION_PARAMS     0x8B
#define LORA_CMD_SET_PACKET_PARAMS         0x8C
#define LORA_CMD_SET_CAD_PARAMS            0x88
#define LORA_CMD_SET_BUFFER_BASE_ADDRESS   0x8F
#define LORA_CMD_SET_LORA_SYMB_NUM_TIMEOUT 0xA0

#define LORA_CMD_GET_STATUS           0xC0
#define LORA_CMD_GET_RSSI_INST        0x15
#define LORA_CMD_GET_RX_BUFFER_STATUS 0x13
#define LORA_CMD_GET_PACKET_STATUS    0x14
#define LORA_CMD_GET_DEVICE_ERRORS    0x17
#define LORA_CMD_CLEAR_DEVICE_ERRORS  0x07
#define LORA_GET_STATS                0x10
#define LORA_RESET_STATS              0x00

#define LORA_REG_SYNC_WORD_MSB 0x0740
#define LORA_REG_SYNC_WORD_LSB 0x0741
#define LORA_REG_RNG0          0x0819
#define LORA_REG_RNG1          0x081A
#define LORA_REG_RNG2          0x081B
#define LORA_REG_RNG3          0x081C
#define LORA_REG_RX_GAIN       0x08AC

#define LORA_SF_5  0x05
#define LORA_SF_6  0x06
#define LORA_SF_7  0x07
#define LORA_SF_8  0x08
#define LORA_SF_9  0x09
#define LORA_SF_10 0x0A
#define LORA_SF_11 0x0B
#define LORA_SF_12 0x0C

#define LORA_BW_7   0x00
#define LORA_BW_10  0x08
#define LORA_BW_15  0x01
#define LORA_BW_20  0x09
#define LORA_BW_31  0x02
#define LORA_BW_41  0x0A
#define LORA_BW_62  0x03
#define LORA_BW_125 0x04
#define LORA_BW_250 0x05
#define LORA_BW_500 0x06

#define LORA_CR_4_5 0x01
#define LORA_CR_4_6 0x02
#define LORA_CR_4_7 0x03
#define LORA_CR_4_8 0x04

#define LORA_RAMP_TIME_10U   0x00
#define LORA_RAMP_TIME_20U   0x01
#define LORA_RAMP_TIME_40U   0x02
#define LORA_RAMP_TIME_80U   0x03
#define LORA_RAMP_TIME_200U  0x04
#define LORA_RAMP_TIME_800U  0x05
#define LORA_RAMP_TIME_1700U 0x06
#define LORA_RAMP_TIME_3400U 0x07

#define LORA_IRQ_TX_DONE           (1 << 0)
#define LORA_IRQ_RX_DONE           (1 << 1)
#define LORA_IRQ_PREAMBLE_DETECTED (1 << 2)
#define LORA_IRQ_SYNC_WORD_VALID   (1 << 3)
#define LORA_IRQ_HEADER_VALID      (1 << 4)
#define LORA_IRQ_HEADER_ERR        (1 << 5)
#define LORA_IRQ_CRC_ERR           (1 << 6)
#define LORA_IRQ_CAD_DONE          (1 << 7)
#define LORA_IRQ_CAD_DETECTED      (1 << 8)
#define LORA_IRQ_TIMEOUT           (1 << 9)
#define LORA_IRQ_ALL               (0xFFFF)

#define LORA_REGULATOR_LDO   0
#define LORA_REGULATOR_DC_DC 1

#define LORA_ERROR_RC64K_CALIB_ERR (1 << 0)
#define LORA_ERROR_RC13M_CALIB_ERR (1 << 1)
#define LORA_ERROR_PLL_CALIB_ERR   (1 << 2)
#define LORA_ERROR_ADC_CALIB_ERR   (1 << 3)
#define LORA_ERROR_IMG_CALIB_ERR   (1 << 4)
#define LORA_ERROR_XOSC_START_ERR  (1 << 5)
#define LORA_ERROR_PLL_LOCK_ERR    (1 << 6)
#define LORA_ERROR_PA_RAMP_ERR     (1 << 8)
