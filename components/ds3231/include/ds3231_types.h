/*
 * DS3231 Driver for ESP32
 * Copyright (c) 2026 Carlos Solano
 *
 * SPDX-License-Identifier: MIT
 *
 * Datasheet: DS3231.pdf
 */

#ifndef DS3231_TYPES_H
#define DS3231_TYPES_H

#include <stdint.h>

// I2C Identification
#define DS3231_I2C_ADDR        0x68

// Data registers
#define DS3231_REG_SECONDS     0x00
#define DS3231_REG_MINUTES     0x01
#define DS3231_REG_HOURS       0x02
#define DS3231_REG_DAY         0x03
#define DS3231_REG_DATE        0x04
#define DS3231_REG_MONTH       0x05
#define DS3231_REG_YEAR        0x06

// Status & Control registers
#define DS3231_REG_CONTROL     0x0E
#define DS3231_REG_STATUS      0x0F
#define DS3231_STATUS_OSF      (1 << 7) // Oscillator Stop Flag

// Temperature registers
#define DS3231_REG_MSB_TEMP    0x11  // Internal temperature
#define DS3231_REG_TEMP_FRAC   0x12  // Internal temperature

// Conversion helpers - BCD format useb by ds3231
static inline uint8_t bcd2dec(uint8_t bcd)
{
  return ((bcd >> 4) * 10 + (bcd & 0x0F));
}

static inline uint8_t dec2bcd(uint8_t dec)
{
  return ((dec / 10) << 4) | (dec % 10);
}

typedef struct {
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  uint8_t day_of_week;  // 1 - 7
  uint8_t day; // 1 - 31
  uint8_t month;// 1 - 12
  uint16_t year; // e.g. 2000 + 26
} ds3231_time_t;

typedef struct {
  uint8_t i2c_addr;
} ds3231_config_t;


#endif // DS3231_TYPES_H
