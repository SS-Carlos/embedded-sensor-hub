# Embedded Sensor Hub for ESP32

Integrated IoT system combining multiple I2C sensors, OLED display,
RTC timestamp and FreeRTOS multitask architecture.

Built on top of the bare-metal drivers from
[i2c-driver-bare-metal-esp32](https://github.com/SS-Carlos/i2c-driver-bare-metal-esp32)
and the FreeRTOS architecture from
[freertos-sensor-tasks](https://github.com/SS-Carlos/freertos-sensor-tasks).

## Status

🚧 Work in progress

## Planned Features

- BMP280 — temperature and pressure
- MPU6050 — accelerometer and gyroscope
- SHT30 — temperature and humidity
- SSD1306 OLED — real-time data display
- DS3231 RTC — timestamp for every measurement
- FreeRTOS multitask with I2C manager task pattern

## License

MIT © Carlos Solano 2026
