#include <stdio.h>
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"

#include "ssd1306.h"
#include "bmp280.h"
#include "ds3231.h"
#include "mpu6050.h"
#include "sht30.h"

#include "sensor_types.h"
#include "bmp280_task.h"
#include "mpu6050_task.h"
#include "sht30_task.h"
#include "rtc_task.h"
#include "display_task.h"

static const char *TAG = "MAIN";

static QueueHandle_t     data_queue = NULL;
static SemaphoreHandle_t i2c_mutex  = NULL;
static task_params_t     params;

void app_main(void)
{
  vTaskDelay(pdMS_TO_TICKS(500));
  /*--- initialize shared i2c bus ---------*/
  i2c_master_bus_handle_t bus_handle;
  i2c_master_bus_config_t bus_cfg = 
  {
    .i2c_port = I2C_NUM_0,
    .sda_io_num = GPIO_NUM_6,
    .scl_io_num = GPIO_NUM_7,
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt = 0,
    .flags.enable_internal_pullup = true,
  };

  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus_handle));
  ESP_LOGI(TAG, "I2C bus initialized");

  ssd1306_config_t display_config = SSD1306_DEFAULT_CONFIG;
  bmp280_config_t  bmp_config     = BMP280_DEFAULT_CONFIG;
  mpu6050_config_t mpu_config     = MPU6050_DEFAULT_CONFIG;
  sht30_config_t   sht_config     = SHT30_DEFAULT_CONFIG;
  ds3231_config_t  rtc_config     = DS3231_DEFAULT_CONFIG;


  ESP_ERROR_CHECK(ssd1306_init(bus_handle, &display_config));
  ESP_ERROR_CHECK(bmp280_init(bus_handle, &bmp_config));
  ESP_ERROR_CHECK(mpu6050_init(bus_handle, &mpu_config));
  ESP_ERROR_CHECK(sht30_init(bus_handle, &sht_config));
  ESP_ERROR_CHECK(ds3231_init(bus_handle, &rtc_config));

  vTaskDelay(pdMS_TO_TICKS(500));

  if (!ds3231_is_valid()) {
    ds3231_time_t t = {
      .seconds     = 0,
      .minutes     = 30,
      .hours       = 18,
      .day_of_week = 2,
      .day         = 20,
      .month       = 10,
      .year        = 2026
    };

    esp_err_t rtc_err = ds3231_set_time(&t); // Avoid error starting sensor hub
    if (rtc_err != ESP_OK) {
      ESP_LOGE(TAG, "RTC set_time failed: %s", esp_err_to_name(rtc_err));
   }
  }

  vTaskDelay(pdMS_TO_TICKS(500));

  data_queue = xQueueCreate(15, sizeof(sensor_message_t));
  i2c_mutex  = xSemaphoreCreateMutex();

  if (data_queue == NULL || i2c_mutex == NULL) {
    ESP_LOGE(TAG, "Failed to create FreeRTOS resources");
    return;
  }

  params.i2c_mutex = i2c_mutex;
  params.data_queue = data_queue;

  xTaskCreate(bmp280_task,  "bmp280",  4096, &params, 5, NULL);
  xTaskCreate(mpu6050_task, "mpu6050", 4096, &params, 5, NULL);
  xTaskCreate(sht30_task,   "sht30",   4096, &params, 5, NULL);
  xTaskCreate(rtc_task,     "rtc",     4096, &params, 5, NULL);
  xTaskCreate(display_task, "display", 4096, &params, 3, NULL);

  vTaskDelay(pdMS_TO_TICKS(1000));
  
}
