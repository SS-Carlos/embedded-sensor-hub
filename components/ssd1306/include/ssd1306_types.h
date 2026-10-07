#ifndef SSD1306_TYPES_H
#define SSD1306_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// I2C addres deppends on SA0 pin
#define SSD1306_I2C_ADDR_PRIMARY                            0x3C  // SA0 to GND
#define SSD1306_I2C_ADDR_SECONDARY                          0x3D  // SA0 to VCC

// Display dimentions
#define SSD1306_WIDTH            128
#define SSD1306_HEIGHT           64
#define SSD1306_PAGES            8
#define SSD1306_BUFFER_SIZE      (SSD1306_WIDTH * SSD1306_PAGES)  // 1028 bytes

// Control bytes - sendes before command or data
#define SSD1306_CTRL_CMD                                    0x00  // next byte is a command
#define SSD1306_CTRL_DATA                                   0x40  // next byte(s) are data

// Memory addressing modes — used with SSD1306_CMD_MEMORY_ADDRESSING_MODE
#define SSD1306_MEM_ADDR_HORIZONTAL                         0x00
#define SSD1306_MEM_ADDR_VERTICAL                           0x01
#define SSD1306_MEM_ADDR_PAGE                               0x02

// Charge pump values — used with SSD1306_CMD_CHARGE_PUMP_SETTING
#define SSD1306_CHARGE_PUMP_ENABLE                          0x14
#define SSD1306_CHARGE_PUMP_DISABLE                         0x10

// Commands definitions
#define SSD1306_CMD_LOWER_COLUMN_START_ADDRESS              0x00
#define SSD1306_CMD_HIGHER_COLUMN_START_ADDRESS             0x10
#define SSD1306_CMD_MEMORY_ADDRESSING_MODE                  0x20
#define SSD1306_CMD_SET_COLUMN_ADDRESS                      0x21
#define SSD1306_CMD_SET_PAGE_ADDRESS                        0x22
#define SSD1306_CMD_SET_FADE_OUT_AND_BLINKING               0x23
#define SSD1306_CMD_RIGHT_HORIZONTAL_SCROLL                 0x26
#define SSD1306_CMD_LEFT_HORIZONTAL_SCROLL                  0x27
#define SSD1306_CMD_VERTICAL_RIGHT_HORIZONTAL_SCROLL        0x29
#define SSD1306_CMD_VERTICAL_LEFT_HORIZONTAL_SCROLL         0x2A
#define SSD1306_CMD_DEACTIVATE_SCROLL                       0x2E
#define SSD1306_CMD_ACTIVATE_SCROLL                         0x2F
#define SSD1306_CMD_DISPLAY_START_LINE                      0x40
#define SSD1306_CMD_CONTRAST_CONTROL                        0x81
#define SSD1306_CMD_CHARGE_PUMP_SETTING                     0x8D
#define SSD1306_CMD_COLUMN_0_MAPPED_TO_SEG0                 0xA0
#define SSD1306_CMD_COLUMN_127_MAPPED_TO_SEG0               0xA1
#define SSD1306_CMD_VERTICAL_SCROLL_AREA                    0xA3
#define SSD1306_CMD_ENTIRE_DISPLAY_OFF                      0xA4
#define SSD1306_CMD_ENTIRE_DISPLAY_ON                       0xA5
#define SSD1306_CMD_NORMAL_DISPLAY                          0xA6
#define SSD1306_CMD_INVERSE_DISPLAY                         0xA7
#define SSD1306_CMD_MULTIPLEX_RATIO                         0xA8
#define SSD1306_CMD_DISPLAY_OFF                             0xAE
#define SSD1306_CMD_DISPLAY_ON                              0xAF
#define SSD1306_CMD_PAGE_ADDR                               0xB0
#define SSD1306_CMD_SCAN_DIRECTION_COM0_START               0xC0
#define SSD1306_CMD_SCAN_DIRECTION_COMN_1_START             0xC8
#define SSD1306_CMD_DISPLAY_OFFSET                          0xD3
#define SSD1306_CMD_DISPLAY_CLOCK_DIVIDE                    0xD5
#define SSD1306_CMD_SET_ZOOM_IN                             0xD6
#define SSD1306_CMD_PRE_CHARGE_PERIOD                       0xD9
#define SSD1306_CMD_COM_PINS_CONF                           0xDA
#define SSD1306_CMD_COMH_DESELECT_LEVEL                     0xDB
#define SSD1306_CMD_NOP                                     0xE3

// Configuration Struct
typedef struct {
  uint8_t i2c_addr;
  uint8_t width;
  uint8_t height;
} ssd1306_config_t;

#endif // SSD1306_TYPES_H
