#pragma once

#include "hardware/spi.h"

#include "sx126X_defines.h"

typedef struct lora_config_t {
    spi_inst_t* spi;
    uint8_t     miso;
    uint8_t     mosi;
    uint8_t     sck;
    uint8_t     cs;
    uint8_t     rst;
    uint32_t    busy;
    uint32_t    freq;
    uint16_t    syncword;
} lora_config;

typedef struct lora_inst_t {
    lora_config config;

    void (*select)(const struct lora_inst_t*, bool);
    void (*spi_op)(const struct lora_inst_t*, uint8_t*, uint8_t*, size_t);
    void (*write_register)(const struct lora_inst_t*, uint16_t, uint8_t*, size_t);
    void (*read_register)(const struct lora_inst_t*, uint16_t, uint8_t*, size_t);
    void (*write_buffer)(const struct lora_inst_t*, uint8_t, const uint8_t*, size_t);
    void (*read_buffer)(const struct lora_inst_t*, uint8_t, uint8_t*, size_t);
} lora_inst;

bool lora_init(lora_inst* lora, lora_config);

void lora_wait_busy(const lora_inst* lora);

void lora_sleep(const lora_inst* lora);
void lora_standby(const lora_inst* lora);
void lora_tx(const lora_inst* lora, uint32_t timeout);
void lora_rx(const lora_inst* lora, uint32_t timeout);

uint16_t lora_get_errors(const lora_inst* lora);
void     lora_print_errors(const lora_inst* lora);

uint8_t lora_get_status(const lora_inst* lora);
void    lora_print_status(const lora_inst* lora);

uint16_t lora_get_sync_word(const lora_inst* lora);
void     lora_set_sync_word(const lora_inst* lora, uint16_t sync_word);

void lora_set_packet_type(const lora_inst* lora, bool lora_mode);
void lora_set_pa_config(
    const lora_inst* lora, uint8_t pa_duty_cycle, uint8_t hp_max, uint8_t device_select);
void lora_set_modulation_params(
    const lora_inst* lora, uint8_t SF, uint8_t BW, uint8_t CR, bool LDRO);
void lora_set_regulator_mode(const lora_inst* lora, uint8_t reg_mode_param);
void lora_set_frequency(const lora_inst* lora, uint32_t freq);
void lora_set_tx_params(const lora_inst* lora, int8_t power, uint8_t ramp_time);
void lora_set_packet_params(const lora_inst* lora, uint16_t preamble_length, bool implicit_header,
    uint8_t payload_length, bool enable_crc, bool invert_iq);

void    lora_set_buffer_base_address(const lora_inst* lora, uint8_t tx_base, uint8_t rx_base);
void    lora_write_tx_message(const lora_inst* lora, const uint8_t* buf, size_t len);
uint8_t lora_read_message(const lora_inst* lora, uint8_t* buf, uint8_t len);
void    lora_get_rx_buffer_status(
       const lora_inst* lora, uint8_t* payload_length_rx, uint8_t* rx_start_buffer_pointer);
void lora_get_packet_status(
    const lora_inst* lora, int8_t* rssi_pkt, int8_t* snr_pkt, int8_t* signal_rssi_pkt);

void lora_set_dio_irq_params(const lora_inst* lora, uint16_t irq_mask, uint16_t dio1_mask,
    uint16_t dio2_mask, uint16_t dio3_mask);

uint16_t lora_get_irq_status(const lora_inst* lora);
void     lora_clear_irq_status(const lora_inst* lora, uint16_t irq);

bool lora_has_received_packet(const lora_inst* lora);
bool lora_is_packet_valid(const lora_inst* lora);
