# Embedded Sensor Hub for ESP32-C6

Integrated IoT system combining multiple I2C sensors, OLED display,
RTC timestamp and FreeRTOS multitask architecture, running on
ESP32-C6 (RISC-V RV32 architecture).

Built on top of the bare-metal drivers from
[i2c-driver-bare-metal-esp32](https://github.com/SS-Carlos/i2c-driver-bare-metal-esp32)
and the FreeRTOS architecture from
[freertos-sensor-tasks](https://github.com/SS-Carlos/freertos-sensor-tasks).

## Multi-Architecture Support

Same codebase running on both architectures:

| Repository | Chip | Architecture | Toolchain |
|---|---|---|---|
| i2c-driver-bare-metal-esp32 | ESP32-D0WDQ6 | Xtensa LX6 | xtensa-esp32-elf |
| embedded-sensor-hub | ESP32-C6 MAN16 | RISC-V RV32 | riscv32-esp-elf |

## Status

| Component | Status |
|---|---|
| BMP280 — temperature and pressure | ✅ Working |
| MPU6050 — accelerometer and gyroscope | ✅ Working |
| SHT30 — temperature and humidity | ✅ Working |
| SSD1306 OLED — real-time data display | ✅ Working |
| DS3231 RTC — timestamp | 🚧 In progress |
| FreeRTOS I2C manager task pattern | 📋 Planned |

## Components

```
components/
├── bmp280/     — temperature and pressure (Bosch BMP280)
├── mpu6050/    — accelerometer and gyroscope (InvenSense MPU6050)
├── sht30/      — temperature and humidity (Sensirion SHT30)
├── ssd1306/    — 128x64 OLED display driver (Solomon SSD1306)
│   ├── include/
│   │   ├── ssd1306_types.h  — commands, dimensions, config struct
│   │   ├── ssd1306.h        — public API
│   │   └── font_5x8.h       — ASCII font bitmap (5x7 in 5x8 cell)
│   └── src/
│       └── ssd1306.c        — framebuffer, I2C communication, text rendering
└── ds3231/     — RTC timestamp (Maxim DS3231) — in progress
```

## SSD1306 Driver Features

- Framebuffer-based rendering — full 128×64 pixel control
- Text rendering with 5x8 ASCII font (95 characters)
- Hardware pixel addressing using page-based memory layout
- `ssd1306_draw_pixel` — individual pixel control with bitmask operations
- `ssd1306_draw_char` / `ssd1306_draw_string` — text at any page position
- `ssd1306_fill` / `ssd1306_clear` — display control
- Verified on ESP32-C6 with `i2c_master_probe` for device detection

## Hardware

- ESP32-C6 MAN16 (RISC-V RV32, single core 160MHz, 16MB flash)
- BMP280 — I2C address 0x76
- MPU6050 GY-521 — I2C address 0x68
- SHT30 — I2C address 0x44
- SSD1306 OLED 128x64 — I2C address 0x3C
- SDA → GPIO 6 / SCL → GPIO 7

## ESP32-C6 Notes

The ESP32-C6 requires specific I2C configuration compared to the
classic ESP32:

```c
i2c_master_bus_config_t bus_cfg = {
    .glitch_ignore_cnt            = 0,    // must be 0 on C6
    .flags.enable_internal_pullup = true,
};

// Use i2c_master_probe() for device detection — not i2c_master_receive()
esp_err_t ret = i2c_master_probe(bus_handle, addr, pdMS_TO_TICKS(20));
```

A 500ms startup delay is required before initializing I2C devices.

## References

- [SSD1306 Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
- [DS3231 Datasheet](https://datasheets.maximintegrated.com/en/ds/DS3231.pdf)
- [ESP-IDF v6 Documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/)
- [i2c-driver-bare-metal-esp32](https://github.com/SS-Carlos/i2c-driver-bare-metal-esp32)
- [freertos-sensor-tasks](https://github.com/SS-Carlos/freertos-sensor-tasks)

## License

MIT © Carlos Solano 2026
