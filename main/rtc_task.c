#include "ds3231.h"
#include "sensor_types.h"
#include "rtc_task.h"
#include "esp_log.h"

static const char * TAG = "DS3231";

void rtc_task(void *pvParameters)
{
  task_params_t *params = (task_params_t *)pvParameters;
  sensor_message_t msg;
  msg.id = SENSOR_DS3231;

  while (1) {
    if (xSemaphoreTake(params->i2c_mutex, pdMS_TO_TICKS(200)) == pdTRUE) {
      esp_err_t raw_err = ds3231_get_time(&msg.data.rtc);
      xSemaphoreGive(params->i2c_mutex);

      msg.status = classify_esp_error(raw_err);

      if (xQueueSendToBack(params->data_queue, &msg, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Queue full, message dropped");
      }
    } else {
      ESP_LOGW(TAG, "Bus busy cicle skipped");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
