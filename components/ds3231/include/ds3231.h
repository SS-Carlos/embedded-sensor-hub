/*
 * DS3231 Driver for ESP32
 * Copyright (c) 2026 Carlos Solano
 *
 * SPDX-License-Identifier: MIT
 *
 * Datasheet: DS3231.pdf
 */

#ifndef DS3231_H
#define DS3231_H

#include <ds3231_types.h>
#include <esp_err.h>
#include <driver/i2c_master.h>
#include <stdbool.h>

/**
 * Values for general use:
 *  - Default I2C address 0x68
 *
 * Usage:
 *  ds3231_config_t config = DS3231_DEFAULT_CONFIG;
 *  ds3231_init(bus_handle, &config);
 *
 * */

#define DS3231_DEFAULT_CONFIG { \
  .i2c_addr = DS3231_I2C_ADDR  \
}

/**
 * --- ds3231_init --------------------
 *
 * Initializes the ds3231 on the i2c bus
 *
 * Performs in order:
 *  1. Registers the device on the i2c bus
 *  2. Check the initial datetime
 *
 * Must be called once before any ds3231_get_time() call
 *
 * @param bus_handle  Initialized I2C master bus handle from app_main
 * @param config      Pointer to RTC configuration struct
 * @return            ESP_OK on success
 *                    ESP_FAIL if I2C communication fails
 */

esp_err_t ds3231_init(i2c_master_bus_handle_t bus_handle, const ds3231_config_t *config);

/**
 * --- ds3231_get_time ----------------
 *
 * Gets datetime from ds3231
 *
 * Reads time and date registers from ds3231 through i2c bus
 * and store the results in the data struct
 *
 * @param time  Pointer to data struct where date and time
 *              will be stored
 *
 * @return      ESP_OK datetime successfully read
 *              ESP_FAIL error reading datetime.
 *
 * */

esp_err_t ds3231_get_time(ds3231_time_t *time);

/**
 * --- ds3231_set_time ----------------
 *
 * Sets datetime in the ds3231
 *
 * Write the date and hour values in the registers through i2c bus
 * RTC will continue standing datetime if the power supply
 * is connected, else where will be lost
 *
 * @param time  Pointer to data struct where date and time
 *              will be setted
 *
 * @return      ESP_OK Datetime successfully set
 *              ESP_FAIL Error sending datetime
 *
 * @note        All values must be in range defined or be valid values
 *              defined in the driver
 * */

esp_err_t ds3231_set_time(const ds3231_time_t *time);

/**
 * --- ds3231_get_temp ----------------
 * 
 * Gets the temperature measured by ds3231 internal sensor
 *
 * Reads the RTCs temperature registers via i2c bus
 * converts the obtained value to degrees celsius and stores
 * in the variable pointed
 *
 * @param temp  Pointer to float variable where temperature will be stored
 *
 * @return      ESP_OK Temperature read successfully.
 *              ESP_FAIL I2C operation failed or the temperature could
 *              not be obtained
 * */
esp_err_t ds3231_get_temp(float *temp);

/* ── ds3231_is_valid ─────────────────────────────────────────────
 *
 * Checks the OSF (Oscillator Stop Flag) in the status register.
 * Returns false if the oscillator was stopped — time may be invalid.
 * Call after ds3231_init() before trusting the time values.
 *
 * @return  true  if time is valid
 *          false if oscillator was stopped (set time with ds3231_set_time)
 */
bool ds3231_is_valid(void);

#endif // DS3231_H
