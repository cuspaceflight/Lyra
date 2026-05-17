#include "sx126X.h"
#include "sx126X_defines.h"

#include "pico/stdlib.h"

#include <stdio.h>

#include <string.h>

#include <hardware/gpio.h>

#include "logger/logger.h"

#define COMBINE_UINT8_2(hi, lo) ((uint16_t)(hi) << 8) | ((uint16_t)(lo))

void lora_spi_select(const lora_inst* lora, bool enable)
{
    LOG_VERBOSE("LORA", "%d", !enable);

    gpio_put(lora->config.cs, !enable);
}

void lora_spi_op(const lora_inst* lora, uint8_t* tx, uint8_t* rx, size_t len)
{
    LOG_VERBOSE("LORA", "%d", len);

    lora_wait_busy(lora);
    lora->select(lora, true);
    spi_write_read_blocking(lora->config.spi, tx, rx, len);
    lora->select(lora, false);
    lora_wait_busy(lora);
}

void lora_spi_write_register(const lora_inst* lora, uint16_t reg, uint8_t* data, size_t len)
{
    LOG_VERBOSE("LORA", "->%d:%d", reg, len);

    uint8_t tx[len + 3];
    uint8_t rx[len + 3];

    tx[0] = LORA_MEM_WRITE_REGISTER;
    tx[1] = (reg >> 8) & 0xFF;
    tx[2] = reg & 0xFF;
    memcpy(tx + 3, data, len);

    lora->spi_op(lora, tx, rx, len + 3);
}

void lora_spi_read_register(const lora_inst* lora, uint16_t reg, uint8_t* data, size_t len)
{
    LOG_VERBOSE("LORA", "<-%d:%d", reg, len);

    uint8_t tx[len + 4];
    uint8_t rx[len + 4];

    tx[0] = LORA_MEM_READ_REGISTER;
    tx[1] = (reg >> 8) & 0xFF;
    tx[2] = reg & 0xFF;
    memset(tx + 3, 0, len + 1);

    lora->spi_op(lora, tx, rx, len + 4);
    memcpy(data, rx + 4, len);
}

void lora_spi_write_buffer(const lora_inst* lora, uint8_t offset, const uint8_t* data, size_t len)
{
    LOG_VERBOSE("LORA", "->%d", offset);

    uint8_t tx[len + 2];
    uint8_t rx[len + 2];

    tx[0] = LORA_MEM_WRITE_BUFFER;
    tx[1] = offset;
    memcpy(tx + 2, data, len);

    lora->spi_op(lora, tx, rx, len + 2);
}

void lora_spi_read_buffer(const lora_inst* lora, uint8_t offset, uint8_t* data, size_t len)
{
    LOG_VERBOSE("LORA", "<-%d", offset);

    uint8_t tx[len + 3];
    uint8_t rx[len + 3];

    tx[0] = LORA_MEM_READ_BUFFER;
    tx[1] = offset;
    memset(tx + 2, 0, len + 1);

    lora->spi_op(lora, tx, rx, len + 3);

    memcpy(data, rx + 3, len);
}

bool lora_init(lora_inst* lora, lora_config config)
{
    LOG_DEBUG("LORA", "");

    if (lora == NULL)
        return false;

    lora->config = config;

    spi_init(lora->config.spi, 500 * 1000);
    gpio_set_function(lora->config.miso, GPIO_FUNC_SPI);
    gpio_set_function(lora->config.mosi, GPIO_FUNC_SPI);
    gpio_set_function(lora->config.sck, GPIO_FUNC_SPI);

    gpio_init(lora->config.cs);
    gpio_set_dir(lora->config.cs, GPIO_OUT);
    gpio_put(lora->config.cs, 1);

    gpio_init(lora->config.rst);
    gpio_set_dir(lora->config.rst, GPIO_OUT);
    gpio_put(lora->config.rst, 1);

    gpio_init(lora->config.busy);
    gpio_set_dir(lora->config.busy, GPIO_IN);

    spi_set_format(lora->config.spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    if (lora->select == NULL) {
        lora->select = lora_spi_select;
    }
    if (lora->spi_op == NULL) {
        lora->spi_op = lora_spi_op;
    }
    if (lora->write_register == NULL) {
        lora->write_register = lora_spi_write_register;
    }
    if (lora->read_register == NULL) {
        lora->read_register = lora_spi_read_register;
    }
    if (lora->write_buffer == NULL) {
        lora->write_buffer = lora_spi_write_buffer;
    }
    if (lora->read_buffer == NULL) {
        lora->read_buffer = lora_spi_read_buffer;
    }

    gpio_put(lora->config.rst, 0);
    sleep_ms(1);
    gpio_put(lora->config.rst, 1);

    lora_standby(lora);

    if (lora_get_sync_word(lora) != 0x1424) {
        return false;
    }
    lora_set_packet_type(lora, true);
    lora_set_pa_config(
        lora, 0x04, 0x07, 0x00); // DUTY CYCLE: 0x04, HP_MAX: 0x07, DEVICE_SELECT: sx1262(0x00)

    lora_set_sync_word(lora, lora->config.syncword);

    lora_set_frequency(lora, lora->config.freq);

    return true;
}

void lora_wait_busy(const lora_inst* lora)
{
    LOG_VERBOSE("LORA", "");
    while (gpio_get(lora->config.busy)) {
        tight_loop_contents();
    }
}

void lora_sleep(const lora_inst* lora)
{
    LOG_DEBUG("LORA", "");

    lora_wait_busy(lora);

    uint8_t buf[2] = {
        LORA_CMD_SET_SLEEP,
        0b00000100, // Warm Start, RTC Timeout disabled
    };
    lora->spi_op(lora, buf, buf, 2);
}

void lora_standby(const lora_inst* lora)
{
    LOG_DEBUG("LORA", "");

    lora_wait_busy(lora);

    uint8_t buf[2] = {
        LORA_CMD_SET_STANDBY,
        0b00000000, /// STDBY_RC
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));
}

void lora_tx(const lora_inst* lora, uint32_t timeout)
{
    LOG_DEBUG("LORA", "0x%08X", timeout);

    lora_wait_busy(lora);

    uint8_t buf[4] = {
        LORA_CMD_SET_TX,
        (timeout >> 16) & 0xFF,
        (timeout >> 8) & 0xFF,
        timeout & 0xFF,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));
}

void lora_rx(const lora_inst* lora, uint32_t timeout)
{
    LOG_DEBUG("LORA", "0x%08X", timeout);

    lora_wait_busy(lora);

    uint8_t buf[4] = {
        LORA_CMD_SET_RX,
        (timeout >> 16) & 0xFF,
        (timeout >> 8) & 0xFF,
        timeout & 0xFF,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));
}

uint16_t lora_get_errors(const lora_inst* lora)
{
    LOG_DEBUG("LORA", "");

    uint8_t buf[4] = {
        LORA_CMD_GET_DEVICE_ERRORS,
        0,
        0,
        0,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));

    return COMBINE_UINT8_2(buf[2], buf[3]);
}

void lora_print_errors(const lora_inst* lora)
{
    LOG_DEBUG("LORA", "");

    uint16_t error = lora_get_errors(lora);
    if (error == 0)
        return;

    if (!(error & LORA_ERROR_RC64K_CALIB_ERR)) {
        printf("LORA_ERROR: RC64K calibration failed\n");
    }
    if (!(error & LORA_ERROR_RC13M_CALIB_ERR)) {
        printf("LORA_ERROR: RC13M calibration failed\n");
    }
    if (!(error & LORA_ERROR_PLL_CALIB_ERR)) {
        printf("LORA_ERROR: PLL calibration failed\n");
    }
    if (!(error & LORA_ERROR_ADC_CALIB_ERR)) {
        printf("LORA_ERROR: ADC calibration failed\n");
    }
    if (!(error & LORA_ERROR_IMG_CALIB_ERR)) {
        printf("LORA_ERROR: IMG calibration failed\n");
    }
    if (!(error & LORA_ERROR_XOSC_START_ERR)) {
        printf("LORA_ERROR: XOSC failed to start\n");
    }
    if (!(error & LORA_ERROR_PLL_LOCK_ERR)) {
        printf("LORA_ERROR: PLL failed to lock\n");
    }
    if (!(error & LORA_ERROR_PA_RAMP_ERR)) {
        printf("LORA_ERROR: PA ramping failed\n");
    }
}

uint8_t lora_get_status(const lora_inst* lora)
{
    LOG_DEBUG("LORA", "");

    lora_wait_busy(lora);

    uint8_t buf[2] = {
        LORA_CMD_GET_STATUS,
        0,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));

    return buf[1];
}

void lora_print_status(const lora_inst* lora)
{
    LOG_DEBUG("LORA", "");

    uint8_t status = lora_get_status(lora);

    uint8_t chip_mode      = (status >> 4) & 0b111;
    uint8_t command_status = (status >> 1) & 0b111;

    printf("MODE: ");
    switch (chip_mode) {
    case 0x2:
        printf("STDBY_RC");
        break;
    case 0x3:
        printf("STDBY_XOSC");
        break;
    case 0x4:
        printf("FS");
        break;
    case 0x5:
        printf("RX");
        break;
    case 0x6:
        printf("TX");
        break;
    }

    printf(" STATUS: ");
    switch (command_status) {
    case 0x2:
        printf("Data is available to host");
        break;
    case 0x3:
        printf("Command timeout");
        break;
    case 0x4:
        printf("Command processing error");
        break;
    case 0x5:
        printf("Failure to execute command");
        break;
    case 0x6:
        printf("Command TX Done");
        break;
    }
    printf("\n");
}

uint16_t lora_get_sync_word(const lora_inst* lora)
{
    LOG_DEBUG("LORA", "");

    lora_wait_busy(lora);

    uint8_t values[2];
    lora->read_register(lora, LORA_REG_SYNC_WORD_MSB, values, 2);
    return COMBINE_UINT8_2(values[0], values[1]);
}

void lora_set_sync_word(const lora_inst* lora, uint16_t sync_word)
{
    LOG_DEBUG("LORA", "0x%04X", sync_word);

    lora_wait_busy(lora);

    uint8_t values[2] = { (sync_word >> 8) & 0xFF, sync_word & 0xFF };
    lora->write_register(lora, LORA_REG_SYNC_WORD_MSB, values, 2);
}

void lora_set_packet_type(const lora_inst* lora, bool lora_mode)
{
    LOG_DEBUG("LORA", "%d", lora_mode);

    lora_wait_busy(lora);

    uint8_t buf[2] = {
        LORA_CMD_SET_PACKET_TYPE,
        lora_mode,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));
}

void lora_set_pa_config(
    const lora_inst* lora, uint8_t pa_duty_cycle, uint8_t hp_max, uint8_t device_select)
{
    LOG_DEBUG(
        "LORA", "Duty Cycle: %d | Hp Max: %d | Device: %d", pa_duty_cycle, hp_max, device_select);

    lora_wait_busy(lora);

    uint8_t buf[5] = {
        LORA_CMD_SET_PA_CONFIG,
        pa_duty_cycle,
        hp_max,
        device_select,
        0x01,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));
}

void lora_set_modulation_params(
    const lora_inst* lora, uint8_t SF, uint8_t BW, uint8_t CR, bool LDRO)
{
    LOG_DEBUG("LORA", "Spreading Factor: %d | Bandwidth: %d | Coding Rate: %d | LDRO: %d", SF, BW,
        CR, LDRO);

    lora_wait_busy(lora);

    uint8_t buf[9] = {
        LORA_CMD_SET_MODULATION_PARAMS,
        SF,
        BW,
        CR,
        LDRO,
        0,
        0,
        0,
        0,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));
}

void lora_set_regulator_mode(const lora_inst* lora, uint8_t reg_mode_param)
{
    LOG_DEBUG("LORA", "Regulator Mode: %d", reg_mode_param);

    lora_wait_busy(lora);

    uint8_t buf[2] = {
        LORA_CMD_SET_REGULATOR_MODE,
        reg_mode_param,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));
}

void lora_set_frequency(const lora_inst* lora, uint32_t freq_hz)
{
    LOG_DEBUG("LORA", "Frequency: %d", freq_hz);

    lora_wait_busy(lora);

    if (freq_hz >= 863E6 && freq_hz <= 870E6) {
        uint8_t buf2[3] = {
            LORA_CMD_CALIBRATE_IMAGE,
            0xD7,
            0xD8,
        };
        lora->spi_op(lora, buf2, buf2, sizeof(buf2));
    } else if (freq_hz >= 902E6 && freq_hz <= 928E6) {
        uint8_t buf2[3] = {
            LORA_CMD_CALIBRATE_IMAGE,
            0xE1,
            0xE9,
        };
        lora->spi_op(lora, buf2, buf2, sizeof(buf2));
    }

    uint64_t rf_freq = ((uint64_t)freq_hz) << 25;
    rf_freq /= 32000000ULL;

    uint8_t buf[5] = {
        LORA_CMD_SET_RF_FREQUENCY,
        (rf_freq >> 24) & 0xFF,
        (rf_freq >> 16) & 0xFF,
        (rf_freq >> 8) & 0xFF,
        (rf_freq >> 0) & 0xFF,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));
}

void lora_set_tx_params(const lora_inst* lora, int8_t power, uint8_t ramp_time)
{
    LOG_DEBUG("LORA", "Power: %d | Ramp Time: %d", power, ramp_time);

    lora_wait_busy(lora);

    power = (power < -9) ? -9 : ((power > 22) ? 22 : power);

    uint8_t buf[3] = {
        LORA_CMD_SET_TX_PARAMS,
        power,
        ramp_time,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));
}

void lora_set_packet_params(const lora_inst* lora, uint16_t preamble_length, bool implicit_header,

    uint8_t payload_length, bool enable_crc, bool invert_iq)
{
    LOG_DEBUG("LORA",
        "Preamble: %d | Implicit Header: %d| Payload Length: %d | CRC: %d | Invert IQ: %d",
        preamble_length, implicit_header, payload_length, enable_crc, invert_iq);

    lora_wait_busy(lora);

    uint8_t buf[10] = {
        LORA_CMD_SET_PACKET_PARAMS,
        (preamble_length >> 8) & 0xFF,
        preamble_length & 0xFF,
        implicit_header,
        payload_length,
        enable_crc,
        invert_iq,
        0,
        0,
        0,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));
}

void lora_set_buffer_base_address(const lora_inst* lora, uint8_t tx_base, uint8_t rx_base)
{
    LOG_DEBUG("LORA", "TX: %d | RX: %d", tx_base, rx_base);

    lora_wait_busy(lora);
    uint8_t buf[3] = {
        LORA_CMD_SET_BUFFER_BASE_ADDRESS,
        tx_base,
        rx_base,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));
}

void lora_write_tx_message(const lora_inst* lora, const uint8_t* buf, size_t len)
{
    LOG_DEBUG("LORA", "%d", len);

    lora_set_buffer_base_address(lora, 0, 0);

    lora_wait_busy(lora);
    lora->write_buffer(lora, 0, buf, len);
}

uint8_t lora_read_message(const lora_inst* lora, uint8_t* buf, uint8_t len)
{
    LOG_DEBUG("LORA", "%d", len);

    uint8_t msg_len, offset;
    lora_get_rx_buffer_status(lora, &msg_len, &offset);
    if (msg_len > len)
        msg_len = len;

    lora->read_buffer(lora, offset, buf, msg_len);

    return msg_len;
}

void lora_get_rx_buffer_status(
    const lora_inst* lora, uint8_t* payload_length_rx, uint8_t* rx_start_buffer_pointer)
{
    LOG_DEBUG("LORA", "");

    uint8_t buf[4] = {
        LORA_CMD_GET_RX_BUFFER_STATUS,
        0,
        0,
        0,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));

    if (payload_length_rx != NULL)
        *payload_length_rx = buf[2];
    if (rx_start_buffer_pointer != NULL)
        *rx_start_buffer_pointer = buf[3];
}

void lora_get_packet_status(
    const lora_inst* lora, int8_t* rssi_pkt, int8_t* snr_pkt, int8_t* signal_rssi_pkt)
{
    LOG_DEBUG("LORA", "");

    uint8_t buf[5] = {
        LORA_CMD_GET_RX_BUFFER_STATUS,
        0,
        0,
        0,
        0,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));

    if (rssi_pkt != NULL)
        *rssi_pkt = -buf[2] / 2;
    if (snr_pkt != NULL)
        *snr_pkt = -buf[3] / 4;
    if (signal_rssi_pkt != NULL)
        *signal_rssi_pkt = -buf[4] / 2;
}

void lora_set_dio_irq_params(const lora_inst* lora, uint16_t irq_mask, uint16_t dio1_mask,
    uint16_t dio2_mask, uint16_t dio3_mask)
{
    LOG_DEBUG("LORA", "IRQ: 0x%02X | DIO1: 0x%02X | DIO2: 0x%02X | DIO3: 0x%02X", irq_mask,
        dio1_mask, dio2_mask, dio3_mask);

    lora_wait_busy(lora);

    uint8_t buf[9] = {
        LORA_CMD_SET_DIO_IRQ_PARAMS,
        (irq_mask >> 8) & 0xFF,
        irq_mask & 0xFF,
        (dio1_mask >> 8) & 0xFF,
        dio1_mask & 0xFF,
        (dio2_mask >> 8) & 0xFF,
        dio2_mask & 0xFF,
        (dio3_mask >> 8) & 0xFF,
        dio3_mask & 0xFF,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));
}

uint16_t lora_get_irq_status(const lora_inst* lora)
{
    LOG_INFO("LORA", "");

    lora_wait_busy(lora);

    uint8_t buf[4] = {
        LORA_CMD_GET_IRQ_STATUS,
        0,
        0,
        0,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));

    return COMBINE_UINT8_2(buf[2], buf[3]);
}

void lora_clear_irq_status(const lora_inst* lora, uint16_t irq)
{
    LOG_INFO("LORA", "IRQ: 0x%02X", irq);

    lora_wait_busy(lora);

    uint8_t buf[3] = {
        LORA_CMD_CLEAR_IRQ_STATUS,
        (irq >> 8) & 0xFF,
        irq & 0xFF,
    };
    lora->spi_op(lora, buf, buf, sizeof(buf));
}

bool lora_has_received_packet(const lora_inst* lora)
{
    LOG_INFO("LORA", "");

    return (lora_get_irq_status(lora) & LORA_IRQ_RX_DONE);
}

bool lora_is_packet_valid(const lora_inst* lora)
{
    LOG_DEBUG("LORA", "");

    return !(lora_get_irq_status(lora) & LORA_IRQ_CRC_ERR);
}
