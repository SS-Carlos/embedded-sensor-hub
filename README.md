# Embedded Sensor Hub for ESP32-C6

Multi-sensor IoT node running on the ESP32-C6 (RISC-V). Three sensors and a
real-time clock share a single I2C bus, each handled by its own FreeRTOS task,
and a paged OLED interface shows the data live together with a sensor health
page.

Built on top of the bare-metal drivers from
[i2c-driver-bare-metal-esp32](https://github.com/SS-Carlos/i2c-driver-bare-metal-esp32)
and the FreeRTOS architecture from
[freertos-sensor-tasks](https://github.com/SS-Carlos/freertos-sensor-tasks).

## Status

| Component | Status |
|---|---|
| BMP280 — temperature and pressure | ✅ Working |
| MPU6050 — accelerometer and gyroscope | ✅ Working |
| SHT30 — temperature and humidity | ✅ Working |
| DS3231 — real-time clock | ✅ Working |
| SSD1306 OLED — paged UI | ✅ Working |
| Sensor health page | ✅ Working |
| I2C manager task pattern | 📋 Planned |
| MicroSD data logger (SPI) | 📋 Planned |

## Architecture

```
bmp280_task  ──┐
mpu6050_task ──┤                         ┌────────────────┐
sht30_task   ──┼──► data_queue ────────► │  display_task  │──► SSD1306
rtc_task     ──┘   (sensor_message_t)    └────────────────┘

All tasks take i2c_mutex before touching the shared I2C bus.
```

- **One producer task per sensor.** Each task reads its sensor under the I2C
  mutex and sends a `sensor_message_t` to a shared queue.
- **One consumer.** `display_task` drains the queue, keeps the latest value of
  each sensor in a cache and redraws the screen at a fixed rate.
- **Every message carries the result of the last transaction** (`esp_err_t`),
  which feeds the health page.
- A task that finds the bus busy skips its cycle instead of reporting a sensor
  error, so a congested bus does not look like a failed sensor.

## OLED interface

A fixed header shows the time from the DS3231 and the current page. The pages
rotate automatically every 4 seconds:

| Page | Content |
|---|---|
| 1 | SHT30 — temperature and humidity |
| 2 | BMP280 — pressure and temperature |
| 3 | MPU6050 — acceleration (X, Y, Z) |
| 4 | Sensor health |

## Sensor health page

Each producer task attaches the result of its last I2C transaction to every
message, and the display task turns that into a per-sensor state. A sensor that
responds normally is shown as healthy; transaction errors, corrupted data
(CRC mismatch on the SHT30) and sensors that stop responding are reported
separately.

## Hardware

| Device | I2C address | Notes |
|---|---|---|
| SSD1306 OLED 128x64 | 0x3C | SA0 to GND |
| DS3231 RTC | 0x68 | Fixed address |
| MPU6050 (GY-521) | 0x69 | AD0 to VCC, moved from 0x68 to avoid the DS3231 |
| BMP280 | 0x76 | SDO to GND |
| SHT30 | 0x44 | ADDR to GND |

- Board: ESP32-C6 MAN16 (RISC-V RV32, single core 160 MHz, 16 MB flash)
- SDA → GPIO 6 / SCL → GPIO 7 (shared bus)

## ESP32-C6 notes

The C6 needs a slightly different I2C setup than the classic ESP32:

```c
i2c_master_bus_config_t bus_cfg = {
    .glitch_ignore_cnt            = 0,    // 0 on the C6
    .flags.enable_internal_pullup = true,
};

// Device detection with i2c_master_probe()
esp_err_t ret = i2c_master_probe(bus_handle, addr, pdMS_TO_TICKS(20));
```

A 500 ms delay before initializing the bus gives the devices time to power up.

## Project structure

```
embedded-sensor-hub/
├── components/
│   ├── bmp280/     — Bosch BMP280 driver
│   ├── mpu6050/    — InvenSense MPU6050 driver
│   ├── sht30/      — Sensirion SHT30 driver (CRC-8 verified)
│   ├── ds3231/     — Maxim DS3231 RTC driver
│   └── ssd1306/    — SSD1306 OLED driver (framebuffer + 5x8 font)
└── main/
    ├── embedded-sensor-hub.c  — bus init, sensor init, task launch
    ├── sensor_types.h         — shared message and parameter structs
    ├── *_task.c/h             — one producer task per sensor
    └── display_task.c/h       — queue consumer and UI
```

## Validation

Tested with all five devices on one bus. Continuous operation: XX h without
failures. Bus signals were inspected with a logic analyzer (PulseView/sigrok).

## Roadmap

- I2C manager task: a single task that owns the bus and serves requests from
  the others via queue
- Button to change page manually
- MicroSD data logger over SPI with RTC timestamps

## References

- [SSD1306 Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
- [DS3231 Datasheet](https://datasheets.maximintegrated.com/en/ds/DS3231.pdf)
- [ESP-IDF v6 Documentation (ESP32-C6)](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/)
- [i2c-driver-bare-metal-esp32](https://github.com/SS-Carlos/i2c-driver-bare-metal-esp32)
- [freertos-sensor-tasks](https://github.com/SS-Carlos/freertos-sensor-tasks)

## License

MIT © Carlos Solano 2026