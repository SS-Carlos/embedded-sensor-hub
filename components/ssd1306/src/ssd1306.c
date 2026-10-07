/*
 * SSD1306 Driver for ESP32
 * Copyright (c) 2026 Carlos Solano
 * SPDX-License-Identifier: MIT
 * Datasheet: SSD1306.pdf
 */

#include "ssd1306.h"
#include "font_5x8.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "SSD1306";

/* ═══════════════════════════════════════════════════════════════
 * PRIVATE INSTANCES
 * ═══════════════════════════════════════════════════════════════ */

static i2c_master_dev_handle_t dev_handle;
static uint8_t framebuffer[SSD1306_BUFFER_SIZE];

/* ═══════════════════════════════════════════════════════════════
 * PRIVATE FUNCTIONS
 * ═══════════════════════════════════════════════════════════════ */

/* ssd1306_send_command
 * Sends a single command byte to the display.
 * Control byte 0x00 indicates the following byte is a command.
 * Datasheet section 8.1.5
 */
static esp_err_t ssd1306_send_command(uint8_t cmd)
{
    uint8_t buf[2] = {SSD1306_CTRL_CMD, cmd};
    return i2c_master_transmit(dev_handle, buf, 2,
                               pdMS_TO_TICKS(100));
}

static esp_err_t ssd1306_send_commands(const uint8_t *cmds, size_t len)
{
    uint8_t buf[len + 1];
    buf[0] = SSD1306_CTRL_CMD;
    memcpy(buf + 1, cmds, len);
    return i2c_master_transmit(dev_handle, buf, len + 1,
                               pdMS_TO_TICKS(100));
}
/* ssd1306_send_data
 * Sends a buffer of data bytes to the display GDDRAM.
 * Control byte 0x40 indicates the following bytes are data.
 * Datasheet section 8.1.5
 */
static esp_err_t ssd1306_send_data(const uint8_t *data, size_t len)
{
    // Allocate buffer with control byte prepended
    uint8_t *buf = malloc(len + 1);
    if (buf == NULL) return ESP_ERR_NO_MEM;

    buf[0] = SSD1306_CTRL_DATA;
    memcpy(buf + 1, data, len);

    esp_err_t ret = i2c_master_transmit(dev_handle, buf, len + 1,
                                         pdMS_TO_TICKS(500));
    free(buf);
    return ret;
}

/* ═══════════════════════════════════════════════════════════════
 * PUBLIC API
 * ═══════════════════════════════════════════════════════════════ */

esp_err_t ssd1306_init(i2c_master_bus_handle_t bus_handle,
                        const ssd1306_config_t *config)
{
    /* 1. Register device on I2C bus */
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = config->i2c_addr,
        .scl_speed_hz    = 100000,
    };
    ESP_RETURN_ON_ERROR(
        i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle),
        TAG, "Failed to add device to I2C bus"
    );

    vTaskDelay(pdMS_TO_TICKS(10));

    /* 2. Send initialization sequence */
    static const uint8_t init_seq[] = {
        SSD1306_CMD_DISPLAY_OFF,
        SSD1306_CMD_DISPLAY_CLOCK_DIVIDE,  0x80,
        SSD1306_CMD_MULTIPLEX_RATIO,       0x3F,
        SSD1306_CMD_DISPLAY_OFFSET,        0x00,
        SSD1306_CMD_DISPLAY_START_LINE,
        SSD1306_CMD_CHARGE_PUMP_SETTING,   SSD1306_CHARGE_PUMP_ENABLE,
        SSD1306_CMD_MEMORY_ADDRESSING_MODE,SSD1306_MEM_ADDR_HORIZONTAL,
        SSD1306_CMD_COLUMN_127_MAPPED_TO_SEG0,
        SSD1306_CMD_SCAN_DIRECTION_COMN_1_START,
        SSD1306_CMD_COM_PINS_CONF,         0x12,
        SSD1306_CMD_CONTRAST_CONTROL,      0xCF,
        SSD1306_CMD_PRE_CHARGE_PERIOD,     0xF1,
        SSD1306_CMD_COMH_DESELECT_LEVEL,   0x40,
        SSD1306_CMD_ENTIRE_DISPLAY_OFF,
        SSD1306_CMD_NORMAL_DISPLAY,
        SSD1306_CMD_DISPLAY_ON
    };

    for (size_t i = 0; i < sizeof(init_seq); i++) {
        ESP_RETURN_ON_ERROR(
            ssd1306_send_command(init_seq[i]),
            TAG, "Failed to send init command"
        );
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    /* 3. Clear framebuffer and update display */
    ssd1306_fill(false);
    ESP_RETURN_ON_ERROR(
        ssd1306_update(),
        TAG, "Failed to clear display on init"
    );

    ESP_LOGI(TAG, "SSD1306 initialized — addr=0x%02X", config->i2c_addr);
    return ESP_OK;
}

esp_err_t ssd1306_display_on(void)
{
    return ssd1306_send_command(SSD1306_CMD_DISPLAY_ON);
}

esp_err_t ssd1306_display_off(void)
{
    return ssd1306_send_command(SSD1306_CMD_DISPLAY_OFF);
}

esp_err_t ssd1306_update(void)
{
    /* Set column address range: 0 to 127 */
    uint8_t col_cmd[] = {
        SSD1306_CMD_SET_COLUMN_ADDRESS, 0, SSD1306_WIDTH - 1
    };
    ESP_RETURN_ON_ERROR(
        ssd1306_send_commands(col_cmd, sizeof(col_cmd)),
        TAG, "Failed to set column address"
    );

    /* Set page address range: 0 to 7 */
    uint8_t page_cmd[] = {
        SSD1306_CMD_SET_PAGE_ADDRESS, 0, SSD1306_PAGES - 1
    };
    ESP_RETURN_ON_ERROR(
        ssd1306_send_commands(page_cmd, sizeof(page_cmd)),
        TAG, "Failed to set page address"
    );

    /* Send framebuffer page by page — 128 bytes per page */
    for (uint8_t page = 0; page < SSD1306_PAGES; page++) {
        ESP_RETURN_ON_ERROR(
            ssd1306_send_data(&framebuffer[page * SSD1306_WIDTH],
                              SSD1306_WIDTH),
            TAG, "Failed to send framebuffer page"
        );
    }

    return ESP_OK;
}
esp_err_t ssd1306_clear(void)
{
    ssd1306_fill(false);
    return ssd1306_update();
}

void ssd1306_fill(bool color)
{
    memset(framebuffer, color ? 0xFF : 0x00, SSD1306_BUFFER_SIZE);
}

void ssd1306_draw_pixel(uint8_t x, uint8_t y, bool color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;

    uint8_t page = y / 8;      // qué página
    uint8_t bit  = y % 8;      // qué bit dentro de la página

    uint16_t index = page * SSD1306_WIDTH + x;

    if (color)
        framebuffer[index] |=  (1 << bit);  // encender pixel
    else
        framebuffer[index] &= ~(1 << bit);  // apagar pixel
}

void ssd1306_draw_char(uint8_t x, uint8_t page, char c)
{
    if (c < FONT_5X8_FIRST_CHAR || c > FONT_5X8_LAST_CHAR) return;
    if (x + FONT_5X8_WIDTH > SSD1306_WIDTH) return;

    uint8_t idx = c - FONT_5X8_FIRST_CHAR;

    for (uint8_t col = 0; col < FONT_5X8_WIDTH; col++) {
        uint16_t index = page * SSD1306_WIDTH + x + col;
        framebuffer[index] = font_5x8_data[idx][col];
    }
}

void ssd1306_draw_string(uint8_t x, uint8_t page, const char *str)
{
    while (*str) {
        if (x + FONT_5X8_WIDTH > SSD1306_WIDTH) break;
        ssd1306_draw_char(x, page, *str);
        x   += FONT_5X8_WIDTH + 1;  // +1 pixel de espaciado entre chars
        str++;
    }
}
