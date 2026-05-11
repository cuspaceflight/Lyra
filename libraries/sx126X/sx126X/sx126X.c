#include "sx126X.h"
#include "sx126X_defines.h"

#include "pico/stdlib.h"

#include <stdio.h>

#include <string.h>

#include <hardware/gpio.h>

#define COMBINE_UINT8_2(hi, lo) ((uint16_t)(hi) << 8) | ((uint16_t)(lo))

#define XTAL_FREQ 32000000UL
#define FREQ_STEP (XTAL_FREQ / (1 << 25))

void lora_spi_select(const lora_inst* lora, bool enable) { gpio_put(lora->config.cs, !enable); }

void lora_spi_op(const lora_inst* lora, uint8_t* tx, uint8_t* rx, size_t len)
{
    lora_wait_busy(lora);
    lora->select(lora, true);
    spi_write_read_blocking(lora->config.spi, tx, rx, len);
    lora->select(lora, false);
    lora_wait_busy(lora);
}

void lora_spi_write_register(const lora_inst* lora, uint16_t reg, uint8_t* data, size_t len)
{
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
    uint8_t tx[len + 2];
    uint8_t rx[len + 2];

    tx[0] = LORA_MEM_WRITE_BUFFER;
    tx[1] = offset;
    memcpy(tx + 2, data, len);

    lora->spi_op(lora, tx, rx, len + 2);
}

void lora_spi_read_buffer(const lora_inst* lora, uint8_t offset, uint8_t* data, size_t len)
{
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

    lora_set_sync_word(lora, lora->config.syncword);
    lora_set_frequency(lora, lora->config.freq);

    lora_set_packet_type(lora, true);

    return true;
}

void lora_wait_busy(const lora_inst* lora)
{
    while (gpio_get(lora->config.busy)) {
        tight_loop_contents();
    }
}

void lora_sleep(const lora_inst* lora)
{
    uint8_t buf[2] = {
        LORA_CMD_SET_SLEEP,
        0b00000100, // Warm Start, RTC Timeout disabled
    };
    lora->spi_op(lora, buf, buf, 2);
}

void lora_standby(const lora_inst* lora)
{
    uint8_t buf[2] = {
        LORA_CMD_SET_STANDBY,
        0b00000000, /// STDBY_RC
    };
    lora->spi_op(lora, buf, buf, 2);
}

void lora_tx(const lora_inst* lora, uint32_t timeout)
{
    uint8_t buf[4] = {
        LORA_CMD_SET_TX,
        (timeout >> 16) & 0xFF,
        (timeout >> 8) & 0xFF,
        timeout & 0xFF,
    };
    lora->spi_op(lora, buf, buf, 4);
}

void lora_rx(const lora_inst* lora, uint32_t timeout)
{
    uint8_t buf[4] = {
        LORA_CMD_SET_RX,
        (timeout >> 16) & 0xFF,
        (timeout >> 8) & 0xFF,
        timeout & 0xFF,
    };
    lora->spi_op(lora, buf, buf, 4);
}

uint16_t lora_get_sync_word(const lora_inst* lora)
{
    uint8_t values[2];
    lora->read_register(lora, LORA_REG_SYNC_WORD_MSB, values, 2);
    return COMBINE_UINT8_2(values[0], values[1]);
}

void lora_set_sync_word(const lora_inst* lora, uint16_t sync_word)
{
    uint8_t values[2] = { (sync_word >> 8) & 0xFF, sync_word & 0xFF };
    lora->write_register(lora, LORA_REG_SYNC_WORD_MSB, values, 2);
}

void lora_set_frequency(const lora_inst* lora, uint32_t freq)
{
    freq = freq * FREQ_STEP;

    uint8_t buf[5] = {
        LORA_CMD_SET_RF_FREQUENCY,
        (freq >> 24) & 0xFF,
        (freq >> 16) & 0xFF,
        (freq >> 8) & 0xFF,
        (freq >> 0) & 0xFF,
    };

    lora->spi_op(lora, buf, buf, 5);
}

void lora_set_packet_type(const lora_inst* lora, bool lora_mode)
{
    uint8_t buf[2] = {
        LORA_CMD_SET_PACKET_TYPE,
        lora_mode,
    };

    lora->spi_op(lora, buf, buf, 2);
}

void lora_set_tx_params(const lora_inst* lora, int8_t power, uint8_t ramp_time)
{
    power = (power < -9) ? -9 : ((power > 22) ? 22 : power);

    uint8_t buf[3] = {
        LORA_CMD_SET_TX_PARAMS,
        power,
        ramp_time,
    };

    lora->spi_op(lora, buf, buf, 3);
}

uint8_t lora_get_status(const lora_inst* lora)
{
    uint8_t buf[2] = {
        LORA_CMD_GET_STATUS,
        0,
    };
    lora->spi_op(lora, buf, buf, 2);

    return buf[1];
}

void lora_set_buffer_base_address(const lora_inst* lora, uint8_t tx_base, uint8_t rx_base)
{
    uint8_t buf[3] = {
        LORA_CMD_SET_BUFFER_BASE_ADDRESS,
        tx_base,
        rx_base,
    };

    lora->spi_op(lora, buf, buf, 3);
}

void lora_write_tx_message(const lora_inst* lora, const uint8_t* buf, size_t len)
{
    lora_set_buffer_base_address(lora, 0, 0);
    lora->write_buffer(lora, 0, buf, len);
}

void lora_set_dio_irq_params(const lora_inst* lora, uint16_t irq_mask, uint16_t dio1_mask,
    uint16_t dio2_mask, uint16_t dio3_mask)
{
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

    lora->spi_op(lora, buf, buf, 9);
}

uint16_t lora_get_irq_status(const lora_inst* lora)
{
    uint8_t buf[4] = {
        LORA_CMD_GET_IRQ_STATUS,
        0,
        0,
        0,
    };

    lora->spi_op(lora, buf, buf, 4);
    return COMBINE_UINT8_2(buf[2], buf[3]);
}

void lora_clear_irq_status(const lora_inst* lora, uint16_t irq)
{
    uint8_t buf[3] = {
        LORA_CMD_CLEAR_IRQ_STATUS,
        (irq >> 8) & 0xFF,
        irq & 0xFF,
    };
    lora->spi_op(lora, buf, buf, 3);
}

void lora_set_modulation_params(
    const lora_inst* lora, uint8_t SF, uint8_t BW, uint8_t CR, bool LDRO)
{
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

    lora->spi_op(lora, buf, buf, 9);
}

void lora_set_packet_params(const lora_inst* lora, uint16_t preamble_length, bool implicit_header,
    uint8_t payload_length, bool enable_crc, bool invert_iq)
{
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

    lora->spi_op(lora, buf, buf, 9);
}
