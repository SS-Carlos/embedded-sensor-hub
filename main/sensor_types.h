#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "bmp280.h"
#include "mpu6050.h"
#include "sht30.h"
#include "ds3231.h"

// Identify which sensor sends the data
typedef enum {
  SENSOR_BMP280,
  SENSOR_MPU6050,
  SENSOR_SHT30,
  SENSOR_DS3231
} sensor_id_t;

typedef enum {
  SENSOR_STATUS_OK = 0,
  SENSOR_STATUS_WARN,
  SENSOR_STATUS_MISSING
} sensor_health_t;

// Global function to classify errors
static inline sensor_health_t classify_esp_error(esp_err_t err)
{
  switch (err) {
  case ESP_OK:
    return SENSOR_STATUS_OK;

  case ESP_ERR_TIMEOUT:
  case ESP_ERR_NOT_FOUND:
    return SENSOR_STATUS_MISSING;

  case ESP_ERR_NO_MEM:
  case ESP_FAIL:
  case ESP_ERR_INVALID_CRC:
  case ESP_ERR_INVALID_ARG:
  case ESP_ERR_INVALID_RESPONSE:
  default:
    return SENSOR_STATUS_WARN; 
  }
}

// mensage which travel trought queue
typedef struct {
  sensor_id_t id;
  esp_err_t   status;
  union {
    bmp280_data_t  bmp;
    mpu6050_data_t mpu;
    sht30_data_t   sht;
    ds3231_time_t  rtc;
  } data;
} sensor_message_t;

// shared resources between tasks
typedef struct {
  SemaphoreHandle_t i2c_mutex;
  QueueHandle_t     data_queue;
} task_params_t;
