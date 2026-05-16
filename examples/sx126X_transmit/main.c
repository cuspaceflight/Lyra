#include "sx126X/sx126X.h"

#include <pico/stdlib.h>
#include <stdio.h>

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

    // Initialize the Lora with some default settings
    if (!lora_init(&lora, lora_config_defaults)) {
        printf("Failed to initialize LoRa");
    }
    // Set the TX parameters
    // Power: 14DBM
    // Ramp time: 200us
    lora_set_tx_params(&lora, 14, LORA_RAMP_TIME_200U);

    // Configure the modulation parameters
    // Spreading Factor: 10
    // Bandwidth: 125
    // Coding Rate: 4/5
    // No Low Data Rate Optimization
    lora_set_modulation_params(&lora, LORA_SF_10, LORA_BW_125, LORA_CR_4_5, false);

    // Enable interrupts for Transmit done and Timeout
    lora_set_dio_irq_params(&lora, LORA_IRQ_TX_DONE | LORA_IRQ_TIMEOUT, 0, 0, 0);

    // Clear any exising interrupts
    lora_clear_irq_status(&lora, LORA_IRQ_ALL);

    bool ready_to_send = true;

    uint32_t len = 50;
    char     message[len];

    uint32_t counter = 0;
    while (true) {
        if (ready_to_send) {
            snprintf(message, len, "Hello, World: %d", counter);

            ready_to_send = false;
            printf("Sending: '%s'\n", message);

            // Write the message into the Lora
            lora_write_tx_message(&lora, (uint8_t*)message, len);

            // Configure the package length and other parameters
            // Preamble Length: 0x08
            // Explicit Header
            // Length of message
            // Enable CRC
            // Disable Invert IQ
            lora_set_packet_params(&lora, 0x08, false, len, true, false);

            // Start the transmission with no timeout
            lora_tx(&lora, 0);
            counter++;
        }

        // Check that the transmit has finished
        if ((lora_get_irq_status(&lora) & LORA_IRQ_TX_DONE) != 0) {
            ready_to_send = true;
            // Clear the status
            lora_clear_irq_status(&lora, LORA_IRQ_TX_DONE);
        }

        sleep_ms(500);
    }
}
