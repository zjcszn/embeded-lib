/*
 * Copyright (c) 2022, Mezael Docoy
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MAIN_BMP280_I2C
#define MAIN_BMP280_I2C

#ifdef __cplusplus
extern "C" {
#endif

#include "bmp280_i2c_hal.h" 

typedef struct{
    uint16_t dig_t1;
    int16_t dig_t2;
    int16_t dig_t3;
    uint16_t dig_p1;
    int16_t dig_p2;
    int16_t dig_p3;
    int16_t dig_p4;
    int16_t dig_p5;
    int16_t dig_p6;
    int16_t dig_p7;
    int16_t dig_p8;
    int16_t dig_p9;
} bmp280_calib_t;

typedef enum{
    T_SB_0_5 = 0x00,
    T_SB_62_5 = 0x01,
    T_SB_125 = 0x02,
    T_SB_250 = 0x03,
    T_SB_5000 = 0x04,
    T_SB_1000 = 0x05,
    T_SB_2000 = 0x06,
    T_SB_4000 = 0x07,
} bmp280_sb_time_t;

typedef enum{
    FILTER_OFF = 0x00,
    FILTER_2 = 0x01,
    FILTER_4 = 0x02,
    FILTER_8 = 0x03,
    FILTER_16 = 0x04,
} bmp280_filter_t;

typedef struct{
    bmp280_sb_time_t t_sb : 3;
    bmp280_filter_t filter : 3;
} bmp280_config_t;

typedef struct{
    uint8_t measuring : 1;
    uint8_t im_update : 1;
} bmp280_status_t;

typedef enum{
    POWERMODE_SLEEP = 0x00,
    POWERMODE_FORCED = 0x01,
    POWERMODE_NORMAL = 0x03,
} bmp280_pwr_mode_t;

typedef enum{
    OSRS_x0 = 0x00,
    OSRS_x1 = 0x01,
    OSRS_x2 = 0x02,
    OSRS_x4 = 0x03,
    OSRS_x8 = 0x04,
    OSRS_x16 = 0x05,
} bmp280_osrs_t;

typedef struct{
    bmp280_osrs_t osrs_tmp : 3;
    bmp280_osrs_t osrs_press : 3;
    bmp280_pwr_mode_t pmode : 2;
} bmp280_ctrl_meas_t;

typedef struct{
    uint32_t pressure;
    int32_t temperature;   
} bmp280_data_t;

/**
 * @brief BMP280 I2C slave address
 */
#define I2C_ADDRESS_BMP280              0x76

/**
 * @brief BMP280 command code registers
 */
#define REG_CALIB                       0x88
#define REG_ID_PARTNUMBER               0xD0
#define REG_RESET                       0xE0
#define REG_STATUS                      0xF3
#define REG_CTRL_MEAS                   0xF4
#define REG_CONFIG                      0xF5
#define REG_PRESS_READ                  0xF7
#define REG_TEMP_READ                   0xFA

/**
 * @brief BMP280 macros
 */
#define RESET_VAL                       0xB6

/**
 * @brief Read BMP280 calibration data
 */
int16_t bmp280_i2c_read_calib(bmp280_calib_t *clb);

/**
 * @brief Setting BMP280 calibration data
 */
int16_t bmp280_i2c_set_calib();

/**
 * @brief Read BMP280 configuration setting
 */
int16_t bmp280_i2c_read_config(uint8_t *cfg);

/**
 * @brief Set BMP280 filter setting
 */
int16_t bmp280_i2c_write_config_filter(bmp280_filter_t fltr);

/**
 * @brief Set BMP280 standby time setting
 */
int16_t bmp280_i2c_write_config_standby_time(bmp280_sb_time_t t_sb);

/**
 * @brief Read BMP280 ctrl_meas register
 */
int16_t bmp280_i2c_read_ctrl_meas(uint8_t *cfg);

/**
 * @brief Set BMP280 power mode setting
 */
int16_t bmp280_i2c_write_power_mode(bmp280_pwr_mode_t pmode);

/**
 * @brief Set BMP280 oversampling setting for temp/presusure
 */
int16_t bmp280_i2c_write_osrs(bmp280_ctrl_meas_t cfg);

/**
 * @brief Read BMP280 status (measuring/updating)
 */
int16_t bmp280_i2c_read_status(bmp280_status_t *sts);

/**
 * @brief Perform BMP280 device reset
 */
int16_t bmp280_i2c_reset();

/**
 * @brief Read BMP280 raw pressure data
 */
int16_t bmp280_i2c_read_pressure_r(int32_t *dt);

/**
 * @brief Read BMP280 raw temperature data
 */
int16_t bmp280_i2c_read_temperature_r(int32_t *dt);

/**
 * @brief Read BMP280 pressure and temperature data
 */
int16_t bmp280_i2c_read_data(bmp280_data_t *dt);

/**
 * @brief Read BMP280 part number
 */
int16_t bmp280_i2c_read_part_number(uint8_t *dt);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_BMP280_I2C */
