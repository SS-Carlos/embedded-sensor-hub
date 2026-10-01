# MPU6050 Driver Component
# Copyright (c) 2026 Carlos Solano
# SPDX-License-Identifier: MIT

idf_component_register(
    SRCS
        "src/mpu6050.c"
    INCLUDE_DIRS
        "include"
    REQUIRES
        esp_driver_i2c
        freertos
        esp_common
)
