#include "pico/stdlib.h"
#include <pico/stdio.h>
#include <pico/time.h>
#include <stdio.h>
#include <time.h>

#include "defines.h"

#include "bmp388/bmp388.h"
#include "icm40609D/icm40609D.h"
#include "sx126X/sx126X.h"

#include "logger/logger.h"
#include "sx126X/sx126X_defines.h"

#define DEBUG

#define CRITICAL_ERROR(MSG)                                                                        \
    while (1) {                                                                                    \
        LOG_ERROR("ERR", MSG);                                                                     \
        sleep_ms(100);                                                                             \
    }

#define ARRAY_SIZE(ARR) sizeof(ARR) / sizeof(ARR[0])

typedef struct task_t {
    void (*setup)();
    void (*run)();

    uint64_t delay;
    uint64_t previous;
} task;

lora_inst lora;
bmp_inst  bmp;
icm_inst  icm;

void setupLora(void)
{
    LOG_INFO("MAIN", "");

    lora_config config = {
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

    if (!lora_init(&lora, config)) {
        CRITICAL_ERROR("Failed to init LoRa");
    }

    lora_set_tx_params(&lora, CONFIG_LORA_POWER, CONFIG_LORA_RAMP_TIME);
    lora_set_modulation_params(&lora, CONFIG_LORA_SF, CONFIG_LORA_BW, CONFIG_LORA_CR, false);
    lora_set_dio_irq_params(&lora, LORA_IRQ_RX_DONE | LORA_IRQ_TIMEOUT, 0, 0, 0);
    lora_clear_irq_status(&lora, LORA_IRQ_ALL);
}

void setupBMP(void)
{
    LOG_INFO("MAIN", "");
    bmp_config config = {
        .i2c  = BMP_I2C_PORT,
        .sda  = BMP_SDA,
        .scl  = BMP_SCL,
        .addr = BMP_ADDR,
    };

    if (!bmp_init(&bmp, config)) {
        CRITICAL_ERROR("Failed to load BMP");
    }

    bmp_osr_config(&bmp, CONFIG_BMP_OSR_P, CONFIG_BMP_OSR_T);
    bmp_odr_config(&bmp, CONFIG_BMP_ODR);
    bmp_iir_config(&bmp, CONFIG_BMP_IIR);
    bmp_pwr_config(&bmp, true, true, BMP_MODE_NORMAL);
}

void setupICM(void)
{
    LOG_INFO("MAIN", "");
    icm_config config = {
        .spi  = ICM_SPI_PORT,
        .miso = ICM_MISO,
        .mosi = ICM_MOSI,
        .sck  = ICM_SCK,
        .cs   = ICM_CS,
    };

    if (!icm_init(&icm, config)) {
        CRITICAL_ERROR("Failed to load ICM");
    }

    icm_pwr_mgmt(&icm, CONFIG_ICM_ACCEL_MODE, CONFIG_ICM_GYRO_MODE, false, false);
    icm_accel_config(&icm, CONFIG_ICM_ACCEL_FSR, CONFIG_ICM_ACCEL_ODR);
    icm_gyro_config(&icm, CONFIG_ICM_GYRO_FSR, CONFIG_ICM_GYRO_ODR);
}

void runLora(void) { LOG_INFO("MAIN", ""); }
void runBMP(void) { LOG_INFO("MAIN", ""); }
void runICM(void) { LOG_INFO("MAIN", ""); }

task tasks[] = {
    { .setup = setupLora, .run = runLora, .delay = 500, .previous = 0 },
    { .setup = setupBMP,  .run = runBMP,  .delay = 250, .previous = 0 },
    { .setup = setupICM,  .run = runICM,  .delay = 100, .previous = 0 },
};

void setup(void)
{
#ifdef DEBUG
    stdio_init_all();

    while (!stdio_init_all()) {
        sleep_ms(100);
    }

    LOG_INFO("MAIN", "USB Connected");
#endif

    for (size_t i = 0; i < ARRAY_SIZE(tasks); i++) {
        tasks[i].setup();
    }
}

void loop(void)
{
    absolute_time_t time       = get_absolute_time();
    uint64_t        current_ms = to_ms_since_boot(time);

    while (1) {
        time       = get_absolute_time();
        current_ms = to_ms_since_boot(time);

        for (size_t i = 0; i < ARRAY_SIZE(tasks); i++) {
            if (current_ms - tasks[i].previous >= tasks[i].delay) {
                tasks[i].previous = current_ms;
                tasks[i].run();
            }
        }
    }
}

int main()
{
    setup();
    loop();
}
