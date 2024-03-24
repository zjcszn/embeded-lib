# BMP280 - ESP-IDF

BMP280 i2c and SPI library for ESP-IDF.
ESP-IDF template used for this project: https://github.com/espressif/esp-idf/tree/master/examples/peripherals/i2c/i2c_simple

## Overview

This example demonstrates usage of BMP280 for reading barometeric pressure.

### Hardware Required

To run this example, you should have one ESP32, ESP32-S or ESP32-C based development board as well as a BMP280. The BMP280 is an absolute barometric pressure sensor especially designed for mobile applications. The sensor module is housed in an extremely compact 8-pin metal-lid LGA package with a footprint of only 2.0 × 2.5 mm2 and 0.95 mm package height. Its small dimensions and its low power consumption of 2.7 µA @1Hz allow the implementation in battery driven devices such as mobile phones, GPS modules or watches.. It is easy to operate via a simple I2C command, you can read the datasheet [here](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmp280-ds001.pdf).

#### Pin Assignment:

**Note:** The following pin assignments are used by default, you can change these in the `menuconfig` .

|                  | SDA             | SCL           |
| ---------------- | -------------- | -------------- |
| ESP I2C Master   | I2C_MASTER_SDA | I2C_MASTER_SCL |
| BMP280           | SDA            | SCL            |


For the actual default value of `I2C_MASTER_SDA` and `I2C_MASTER_SCL` see `Example Configuration` in `menuconfig`.

**Note: ** There’s no need to add an external pull-up resistors for SDA/SCL pin, because the driver will enable the internal pull-up resistors.

### Build and Flash

Enter `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

```bash
I (323) example_usage: Part number: 0x58
I (333) example_usage: Calibration data setting: Successful
I (333) example_usage: Setting to normal mode: Successful
I (443) example_usage: BMP280 initialization successful
I (443) example_usage: Pressure: 100607.3 Pa
I (443) example_usage: Temperature: 29.9 °C
I (2443) example_usage: Pressure: 100609.2 Pa
I (2443) example_usage: Temperature: 30.1 °C
I (4443) example_usage: Pressure: 100612.7 Pa
I (4443) example_usage: Temperature: 30.1 °C
I (6443) example_usage: Pressure: 100610.1 Pa
I (6443) example_usage: Temperature: 30.1 °C
I (8443) example_usage: Pressure: 100610.0 Pa
I (8443) example_usage: Temperature: 30.1 °C
I (10443) example_usage: Pressure: 100609.3 Pa
I (10443) example_usage: Temperature: 30.1 °C
I (12443) example_usage: Pressure: 100610.1 Pa
I (12443) example_usage: Temperature: 30.1 °C
I (14443) example_usage: Pressure: 100612.2 Pa
I (14443) example_usage: Temperature: 30.1 °C
I (16443) example_usage: Pressure: 100611.5 Pa
```
