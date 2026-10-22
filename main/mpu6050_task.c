#include "mpu6050.h"
#include "sensor_types.h"
#include "mpu6050_task.h"
#include "esp_log.h"

static const char * TAG = "MPU6050";

void mpu6050_task(void *pvParameters)
{
  task_params_t *params = (task_params_t *)pvParameters;
  sensor_message_t msg;
  msg.id = SENSOR_MPU6050;

  while (1) {
    if (xSemaphoreTake(params->i2c_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      esp_err_t raw_err = mpu6050_read(&msg.data.mpu);
      xSemaphoreGive(params->i2c_mutex);

      msg.status = classify_esp_error(raw_err);

      if (xQueueSendToBack(params->data_queue, &msg, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Queue full, message dropped");
      }
    } else {
      ESP_LOGW(TAG, "Bus busy, cycle skipped");
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
