/*
 * Copyright (c) 2024 UFSM
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>

#define AT30TSE75X_ADDR 0x4F
#define AT30TSE75X_REG_REA 0x00

#define I2C_DEV_NAME sercom1

#define SLEEP_TIME      K_MSEC(1000)

int main(void) {
    const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(I2C_DEV_NAME));

    if (i2c_dev == NULL || !device_is_ready(i2c_dev)) {
        printf("Could not get I2C device\n");
        return 0;
    }

    printf("Sensor ready! Starting readings:\n");

    for (;;) {
        uint8_t temperature;
        int ret = i2c_reg_read_byte(i2c_dev, AT30TSE75X_ADDR, AT30TSE75X_REG_REA, &temperature);

        if (ret != 0) {
            printf("I2C read error: %d\n", ret);
            k_sleep(SLEEP_TIME);
            continue;
        }

        if (temperature > 127 || temperature < -128) {
            printf("Invalid temperature reading: %d\n", temperature);
        } else {
            printf("Temperature: %d C\n", temperature);
        }

        k_sleep(SLEEP_TIME);
    }

    return 0;
}
