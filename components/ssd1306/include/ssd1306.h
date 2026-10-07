#ifndef SSD1306_H
#define SSD1306_H

#include "ssd1306_types.h"
#include "esp_err.h"
#include "driver/i2c_master.h"

/* Default configuration
 *
 * Values for general use:
 *  - Primary I2C address (SA0 to GND)
 *  - width - width on screen
 *  - Height  - Height on screen
 *
 * Usage:
 *  ssd1306_config_t config = SSD1306_DEFAULT_CONFIG;
 *  ssd1306_init(bus_handle, &config);
 */

// SSD1306 Default Config
#define SSD1306_DEFAULT_CONFIG {                \
  .i2c_addr      = SSD1306_I2C_ADDR_PRIMARY,  \
  .width         = SSD1306_WIDTH,             \
  .height        = SSD1306_HEIGHT             \
}

/*==================== Public API ====================*/

/**
 * @brief Driver Initialization, sends harware commands and cleans buffer
 * @param bus_handle Initialized I2C master bus handle
 * @param config Pointer to display configuration struct
 * @return ESP_OK on success
 *         ESP_FAIL if I2C communication fails
 */
esp_err_t ssd1306_init(i2c_master_bus_handle_t bus_handle,
                       const ssd1306_config_t *config);


// Display control

/**
 * @brief Turn the SSD1306 display on
 * Sends commands to enable OLED panel output
 *
 * @return ESP_OK: Display turned on succesfully
 *         Other: I2C communication error
 */
esp_err_t ssd1306_display_on(void);

/**
 * @brief Turn the SSD1306 display off
 * Sends commands to disable the OLED panel output
 *
 * @return ESP_OK: Display turned off succesfully.
 *         Other: I2C communication error.
 */
esp_err_t ssd1306_display_off(void);

/**
 * @brief Clear the display framebuffer.
 *
 * Clears the driver's framebuffer by setting all pixels
 * to the off state.
 *
 * @return ESP_OK: Framebuffer cleared succesfully.
 *         Other: Error reported by the driver.
 */
esp_err_t ssd1306_clear(void);

/**
 * @brief Update the SSD1306 display.
 *
 * Sends content from the local framebuffer to the GDDRAM screen
 * 
 * @return ESP_OK on success.
 *         ESP_FAIL if transmission fails.
 */
esp_err_t ssd1306_update(void);



// Framebuffer / graphics

/**
 * @brief Fill the entire display with specified color.
 *
 * Sets every pixel in the framebuffer to the specified state.
 * The display is not updated until ssd1306_update() is called.
 *
 * @param color: Pixel state to apply to the entire display.
 */
void ssd1306_fill(bool color);

/**
 * @brief Draw a single pixel in the framebuffer.
 *
 * Modifies the specified pixel in the local framebuffer.
 *
 * @param x Horizontal pixel coordinate.
 * @param y Vertical pixel coordinate.
 * @param color Pixel state. true turn the pixels on.
 *
 */
void ssd1306_draw_pixel(uint8_t x, uint8_t y, bool color);

// Text

/**
 * @brief Draw a ASCII Character.
 * @param x starting coordinate.
 * @param page Page starting position.
 * @param c Character to draw.
 */
void ssd1306_draw_char(uint8_t x, uint8_t page, char c);

/**
 * @brief Draw a string.
 * @param x starting coordinate
 * @param page Page starting position.
 * @param str pointer to chain to draw.
 */
void ssd1306_draw_string(uint8_t x, uint8_t page, const char *str);

#endif // SSD1306_H
