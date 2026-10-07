#include "ssd1306.h"
#include "bmp280.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MAIN";

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
  ESP_ERROR_CHECK(ssd1306_init(bus_handle, &display_config));

  bmp280_config_t bmp_config = BMP280_DEFAULT_CONFIG;
  ESP_ERROR_CHECK(bmp280_init(bus_handle, &bmp_config));

  bmp280_data_t bmp_data;

  char line1[20];
  char line2[20];

  while (1) {
    bmp280_read(&bmp_data);

    snprintf(line1, sizeof(line1), "Temp: %.1fC", bmp_data.temperature);
    snprintf(line2, sizeof(line2), "Press: %.fhPa", bmp_data.pressure);

    // basic test.
    ssd1306_clear();
    ssd1306_draw_string(0, 0, line1);
    ssd1306_draw_string(0, 2, line2);
    ssd1306_update();

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
