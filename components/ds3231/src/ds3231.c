/*
 * DS3231 Driver for ESP32
 * Copyright (c) 2026 Carlos Solano
 *
 * SPDX-License-Identifier: MIT
 *
 * Datasheet: DS3231.pdf
 */

#include "ds3231.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"

static const char *TAG = "DS3231";

// Private module instances
static i2c_master_dev_handle_t dev_handle;

// Private functions

/**
 * --- ds3231_read_register -----------
 *
 * Reads the datetime registers from 0x00
 * Uses i2c_master_transmit_receive()
 *
 * @param reg  Register address to read from.
 * @param buf  Buffer to store received bytes.
 * @param len  Number of bytes to read.
 * @return     ESP_OK on success.
 * */

static esp_err_t ds3231_read_register(uint8_t reg, 
                                      uint8_t *buf, 
                                      uint8_t len)
{
  return i2c_master_transmit_receive(dev_handle, 
                                     &reg, 1, 
                                     buf, len, 
                                     pdMS_TO_TICKS(100));
}

/**
 * --- ds3231_write_register ----------
 *
 * Writer to datetime registers to set initial
 * date and hour values
 *
 * Uses i2c_master_transmit()
 *
 * @param reg  Register address to write to.
 * @param buf  Buffer with registers to write.
 * @param len  Number of bytes to write.
 * @return     ESP_OK on success.
 * */

static esp_err_t ds3231_write_register(uint8_t reg, 
                                       const uint8_t *buf, 
                                       uint8_t len)
{
  if (len == 0 || len > 8) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t data[9];
  data[0] = reg;

  for (size_t i = 0; i < len; i++) {
       data[1 + i] = buf[i];
  }

  return i2c_master_transmit(dev_handle, data, len + 1, pdMS_TO_TICKS(100));
}

/**
   --- ds3231_force_24h_mode -----------
   Forces the 24h mode in the RTC DS3231
 
   @return ESP_OK On success
 */

static esp_err_t ds3231_force_24h_mode(void)
{
  uint8_t current_hours = 0;
  
  // Read the actual value in the hour register
  esp_err_t err = ds3231_read_register(DS3231_REG_HOURS, &current_hours, 1);
  if (err != ESP_OK) {
    return err;
  }

  // Check if the bit 6 is 0 (24h mode)
  if (current_hours & (1 << 6)) {
    current_hours &= ~(1 << 6);

    // Write back the modified register.
    err = ds3231_write_register(DS3231_REG_HOURS, &current_hours, 1);
    if (err != ESP_OK) {
      return err;
    }
  }

  return ESP_OK;
}

// Public API

/**
   --- ds3231_is_valid ---------
   Check the oscillator stop flag. if is 1 or 0 return
   a bool.

   @return bool.
 */

bool ds3231_is_valid(void)
{
  uint8_t status = 0;

  if (ds3231_read_register(DS3231_REG_STATUS, &status, 1) != ESP_OK) {
    return false;
  }

  return (status & DS3231_STATUS_OSF) == 0;
}


esp_err_t ds3231_init(i2c_master_bus_handle_t bus_handle, const ds3231_config_t *config)
{
  i2c_device_config_t dev_cfg =
    {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address  = config->i2c_addr,
      .scl_speed_hz    = 400000,
    };

  ESP_RETURN_ON_ERROR(
    i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle),
    TAG, "Failed to add device to I2C bus"
    );

  ESP_RETURN_ON_ERROR(
    ds3231_force_24h_mode(),
    TAG, "Failed to set 24h mode"
    );

  if(!ds3231_is_valid()) {
    ESP_LOGW(TAG, "Oscillator stopped or RTC unreachable - "
             "time is not realiable, call ds3231_set_time()");
  } else {
    ESP_LOGI(TAG, "Oscillator ran without interruption - time is reliable");
  }

  ESP_LOGI(TAG, "DS3231 initialized - addr=0x%02X", config->i2c_addr);

  return ESP_OK;
}


esp_err_t ds3231_get_time(ds3231_time_t *time)
{
  ESP_RETURN_ON_FALSE(time != NULL, ESP_ERR_INVALID_ARG, TAG, "Time is NULL");
  
  uint8_t data[7];

  esp_err_t err = ds3231_read_register(DS3231_REG_SECONDS, data, 7);
  if (err != ESP_OK) return err;

  time->seconds     = bcd2dec(data[0] & 0x7F);
  time->minutes     = bcd2dec(data[1] & 0x7F);
  time->hours       = bcd2dec(data[2] & 0x3F); // 24h mode
  time->day_of_week = bcd2dec(data[3] & 0x07);
  time->day         = bcd2dec(data[4] & 0x3F);
  time->month       = bcd2dec(data[5] & 0x1F);
  time->year        = 2000 + bcd2dec(data[6]);

  return ESP_OK;
}


esp_err_t ds3231_set_time(const ds3231_time_t *time)
{
  ESP_RETURN_ON_FALSE(time != NULL, ESP_ERR_INVALID_ARG, TAG, "Time is NULL");

  // Range validation
  ESP_RETURN_ON_FALSE(time->seconds <= 59 && time->minutes <= 59 &&
                      time->hours   <= 23,
                      ESP_ERR_INVALID_ARG, TAG, "Invalid time of day");
  ESP_RETURN_ON_FALSE(time->day_of_week >= 1 && time->day_of_week <= 7,
                      ESP_ERR_INVALID_ARG, TAG, "Invalid day of week");
  ESP_RETURN_ON_FALSE(time->day >= 1 && time->day <= 31 &&
                      time->month >= 1 && time->month <= 12,
                      ESP_ERR_INVALID_ARG, TAG, "Invalid date");
  ESP_RETURN_ON_FALSE(time->year >= 2000 && time->year <= 2099,
                      ESP_ERR_INVALID_ARG, TAG, "Year out of range");

  // Decimal -> BCD. Hours with bit 6 = 0 keeps the RTC in 24h mode
  uint8_t data[7] = {
    dec2bcd(time->seconds),
    dec2bcd(time->minutes),
    dec2bcd(time->hours),
    dec2bcd(time->day_of_week),
    dec2bcd(time->day),
    dec2bcd(time->month),
    dec2bcd(time->year - 2000)
  };

  // 1. Write all registers in a single transaction from 0x00
  ESP_RETURN_ON_ERROR(
    ds3231_write_register(DS3231_REG_SECONDS, data, sizeof(data)),
    TAG, "Failed to write time"
    );

  // 2. The time is now trustworthy -> clear OSF (read-modify-write)
  uint8_t status = 0;
  ESP_RETURN_ON_ERROR(
    ds3231_read_register(DS3231_REG_STATUS, &status, 1),
    TAG, "Failed to read status register"
    );

  status &= ~DS3231_STATUS_OSF;

  ESP_RETURN_ON_ERROR(
    ds3231_write_register(DS3231_REG_STATUS, &status, 1),
    TAG, "Failed to clear OSF"
    );

  return ESP_OK;
}


esp_err_t ds3231_get_temp(float *temp)
{
  ESP_RETURN_ON_FALSE(temp != NULL, ESP_ERR_INVALID_ARG, TAG, "Temp is NULL");
  
  uint8_t data[2];
  
  esp_err_t err = ds3231_read_register(DS3231_REG_MSB_TEMP, data, 2);
  if (err != ESP_OK) return err;

  int8_t msb = (int8_t)data[0];
  uint8_t lsb = data[1] >> 6;

  *temp = (float)msb + (lsb * 0.25f);
  return ESP_OK;
}
