#include <stdio.h>
#include "display_task.h"
#include "sensor_types.h"
#include "ssd1306.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "DISPLAY";

typedef enum {
  PAGE_ENVIRONMENT = 0,
  PAGE_PRESSURE,
  PAGE_MOTION,
  PAGE_HEALTH,
  PAGE_MAX
} ui_page_t;

typedef struct {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  float temp_sht;
  float humidity;
  float temp_bmp;
  float pressure;
  float accel_x;
  float accel_y;
  float accel_z;
  sensor_health_t bmp280_status;
  sensor_health_t mpu6050_status;
  sensor_health_t sht30_status;
  sensor_health_t ds3231_status;
} system_cache_t;

// Function to convert err to text for OLED use
const char* health_to_string(sensor_health_t health) {
    switch (health) {
        case SENSOR_STATUS_OK:      return "OK";
        case SENSOR_STATUS_WARN:    return "WARN";
        case SENSOR_STATUS_MISSING: return "MISSING";
        default:                    return "UNK";
    }
}

void display_task(void *pvParameters)
{
  task_params_t   *params = (task_params_t *)pvParameters;
  sensor_message_t received;
  system_cache_t cache = {0};

  ui_page_t current_page = PAGE_ENVIRONMENT;
  TickType_t last_page_switch = xTaskGetTickCount();
  const TickType_t page_interval = pdMS_TO_TICKS(4000); // Change page every 4s

  while (1) {
    while (xQueueReceive(params->data_queue, &received, 0) == pdTRUE) {
      switch (received.id) {
        case SENSOR_BMP280:
          cache.temp_bmp = received.data.bmp.temperature;
          cache.pressure = received.data.bmp.pressure;
          cache.bmp280_status = received.status;
        break;
        case SENSOR_MPU6050:
          cache.accel_x = received.data.mpu.accel_x;
          cache.accel_y = received.data.mpu.accel_y; 
          cache.accel_z = received.data.mpu.accel_z;
          cache.mpu6050_status = received.status;
        break;
        case SENSOR_SHT30:
          cache.temp_sht = received.data.sht.temperature; 
          cache.humidity = received.data.sht.humidity;
          cache.sht30_status = received.status;
        break;
        case SENSOR_DS3231:
          cache.hours   = received.data.rtc.hours;
          cache.minutes = received.data.rtc.minutes;
          cache.seconds = received.data.rtc.seconds;
          cache.ds3231_status = received.status;
        break;
     
        default:
          ESP_LOGW(TAG, "Unknown Sensor ID: %d", received.id);
        break;
      }

    }

    if ((xTaskGetTickCount() - last_page_switch) >= page_interval) {
      current_page = (current_page + 1) % PAGE_MAX;
      last_page_switch = xTaskGetTickCount();
    }

    ssd1306_fill(false);

    char header_buf[32];
    snprintf(header_buf, sizeof(header_buf), "%02d:%02d:%02d  [P%d/%d]",
             cache.hours, cache.minutes, cache.seconds, current_page + 1, PAGE_MAX);
    ssd1306_draw_string(0, 0, header_buf);

    // Dinamic page
    char line0[32] = "";
    char line1[32] = "";
    char line2[32] = "";
    char line3[32] = "";

    switch (current_page) {
      case PAGE_ENVIRONMENT:
        
        ssd1306_draw_string(0, 2, "SHT30: ENV");
        snprintf(line1, sizeof(line1), "Temp:  %.1f C", cache.temp_sht);
        snprintf(line2, sizeof(line2), "Humed: %.1f %%", cache.humidity);
        break;
                
      case PAGE_PRESSURE:
        ssd1306_draw_string(0, 2, "BMP280: BARO");
        snprintf(line1, sizeof(line1), "Pres:  %.1f hPa", cache.pressure);
        snprintf(line2, sizeof(line2), "Temp:  %.1f C", cache.temp_bmp);
        break;
               
      case PAGE_MOTION:
        ssd1306_draw_string(0, 2, "MPU6050: IMU");
        snprintf(line1, sizeof(line1), "Ax: %.2f g", cache.accel_x);
        snprintf(line2, sizeof(line2), "Ay: %.2f g", cache.accel_y);
        snprintf(line3, sizeof(line3), "Az: %.2f g", cache.accel_z);
        break;

      case PAGE_HEALTH:
        ssd1306_draw_string(0, 2, "SENSOR STATUS");
        snprintf(line0, sizeof(line0), "bmp280: %s",  health_to_string(cache.bmp280_status));
        snprintf(line1, sizeof(line1), "mpu6050:%s", health_to_string(cache.mpu6050_status));
        snprintf(line2, sizeof(line2), "sht30:  %s",   health_to_string(cache.sht30_status));
        snprintf(line3, sizeof(line3), "ds3231: %s",  health_to_string(cache.ds3231_status));
        break;
                
      default:
        break;
    }

    ssd1306_draw_string(0, 3, line0);
    ssd1306_draw_string(0, 4, line1);
    ssd1306_draw_string(0, 5, line2);
    ssd1306_draw_string(0, 6, line3);

    if (xSemaphoreTake(params->i2c_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      ssd1306_update();
      xSemaphoreGive(params->i2c_mutex);
    }

    vTaskDelay(pdMS_TO_TICKS(300));
  }
}
