#include "pico/stdlib.h"
#include <stdio.h>

#include "sx126X/sx126X.h"

#define LORA_SPI_PORT spi1
#define LORA_MISO     12
#define LORA_MOSI     11
#define LORA_CS       9
#define LORA_SCK      10
#define LORA_BUSY     13
#define LORA_RST      8
#define LORA_DIO1     14

lora_inst lora;

int main()
{
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    lora_config lora_config_defaults = {
        .spi      = LORA_SPI_PORT,
        .miso     = LORA_MISO,
        .mosi     = LORA_MOSI,
        .sck      = LORA_SCK,
        .cs       = LORA_CS,
        .rst      = LORA_RST,
        .busy     = LORA_BUSY,
        .freq     = 868E6,
        .syncword = 0x3444,
    };

    // Intiailize the Lora with some default settings
    if (!lora_init(&lora, lora_config_defaults)) {
        printf("Failed to initialize LoRa");
    }

    // Configure the modulation params
    // Spreading Factor: 10
    // Bandwidth: 125
    // Coding Rate: 4/5
    // No Low Data Rate Optimization
    lora_set_modulation_params(&lora, LORA_SF_10, LORA_BW_125, LORA_CR_4_5, false);

    // Configure the interrupts to report on Receive Done and Timeouts
    lora_set_dio_irq_params(&lora, LORA_IRQ_RX_DONE | LORA_IRQ_TIMEOUT, 0, 0, 0);

    // Clear any existing interrupts
    lora_clear_irq_status(&lora, LORA_IRQ_ALL);

    // Set the packet paramaters,
    // Preamble Length: 8
    // Explicit Header
    // Max payload length: 255
    // Enable CRC
    // Don't Invert IQ
    lora_set_packet_params(&lora, 0x08, false, 0xFF, true, false);

    // Set the base address of RX and TX to 0
    lora_set_buffer_base_address(&lora, 0, 0);

    // Enable continuous RX
    lora_rx(&lora, 0xFFFFFF);

    uint32_t len = 255;
    char     message[len];

    while (true) {
        // Check for available packets
        if (lora_has_received_packet(&lora)) {
            // Verify the CRC is valid
            if (!lora_is_packet_valid(&lora)) {
                printf("Invalid Packet\n");
                continue;
            }

            // Read in the message
            uint8_t read = lora_read_message(&lora, (uint8_t*)message, len);
            printf("RECEIVED: %.*s\n", read, message);

            // Clear the read packet status
            lora_clear_irq_status(&lora, LORA_IRQ_ALL);
        }

        sleep_ms(500);
    }
}
