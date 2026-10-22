#include "sht30.h"
#include "sensor_types.h"
#include "sht30_task.h"
#include "esp_log.h"

static const char * TAG = "SHT30";

void sht30_task( void * pvParamters)
{
  task_params_t *params = (task_params_t *)pvParamters;
  sensor_message_t msg;
  msg.id = SENSOR_SHT30;

  while (1) {
    if (xSemaphoreTake(params->i2c_mutex, pdMS_TO_TICKS(200)) == pdTRUE) {
      esp_err_t raw_err = sht30_read(&msg.data.sht);
      xSemaphoreGive(params->i2c_mutex);

      msg.status = classify_esp_error(raw_err);

      if (xQueueSendToBack(params->data_queue, &msg, 0)) {
        ESP_LOGW(TAG, "Queue full, message dropped");
      }
    } else {
      ESP_LOGW(TAG, "Bus busy, cycle skipped");
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

