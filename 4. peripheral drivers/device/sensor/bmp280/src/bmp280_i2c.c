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

#include "bmp280_i2c.h" 
#include "bmp280_i2c_hal.h" 

bmp280_calib_t calib_params;

int16_t bmp280_i2c_read_calib(bmp280_calib_t *clb)
{
    uint8_t reg = REG_CALIB;
    uint8_t data[24];
    int16_t err = bmp280_i2c_hal_read(I2C_ADDRESS_BMP280, &reg, data, 24);
    clb->dig_t1 = (data[1] << 8)  | data[0];
    clb->dig_t2 = (data[3] << 8)  | data[2];
    clb->dig_t3 = (data[5] << 8)  | data[4];
    clb->dig_p1 = (data[7] << 8)  | data[6];
    clb->dig_p2 = (data[9] << 8)  | data[8];
    clb->dig_p3 = (data[11] << 8) | data[10];
    clb->dig_p4 = (data[13] << 8) | data[12];
    clb->dig_p5 = (data[15] << 8) | data[14];
    clb->dig_p6 = (data[17] << 8) | data[16];
    clb->dig_p7 = (data[19] << 8) | data[18];
    clb->dig_p8 = (data[21] << 8) | data[20];
    clb->dig_p9 = (data[23] << 8) | data[22];
    return err;
}

int16_t bmp280_i2c_set_calib()
{
    int16_t err = bmp280_i2c_read_calib(&calib_params);
    return err;
}

int16_t bmp280_i2c_read_config(uint8_t *cfg)
{
    uint8_t reg = REG_CONFIG;
    int16_t err = bmp280_i2c_hal_read(I2C_ADDRESS_BMP280, &reg, cfg, 1);
    return err;
}

int16_t bmp280_i2c_write_config_filter(bmp280_filter_t fltr)
{
    uint8_t reg = REG_CONFIG;
    uint8_t data[2], cfg;
    int16_t err = bmp280_i2c_read_config(&cfg);
    data[0] = reg;
    data[1] = (cfg & 0xE3) | (fltr << 2);
    err += bmp280_i2c_hal_write(I2C_ADDRESS_BMP280, data, 2);
    return err;
}

int16_t bmp280_i2c_write_config_standby_time(bmp280_sb_time_t t_sb)
{
    uint8_t reg = REG_CONFIG;
    uint8_t data[2], cfg;
    int16_t err = bmp280_i2c_read_config(&cfg);
    data[0] = reg;
    data[1] = (cfg & 0x1F) | (t_sb << 5);
    err += bmp280_i2c_hal_write(I2C_ADDRESS_BMP280, data, 2);
    return err;
}

int16_t bmp280_i2c_read_ctrl_meas(uint8_t *cfg)
{
    uint8_t reg = REG_CTRL_MEAS;
    int16_t err = bmp280_i2c_hal_read(I2C_ADDRESS_BMP280, &reg, cfg, 1);
    return err;
}

int16_t bmp280_i2c_write_power_mode(bmp280_pwr_mode_t pmode)
{
    uint8_t reg = REG_CTRL_MEAS;
    uint8_t cfg, data[2];
    int16_t err = bmp280_i2c_read_ctrl_meas(&cfg);
    data[0] = reg;
    data[1] = (cfg & 0xFC) | pmode;
    err += bmp280_i2c_hal_write(I2C_ADDRESS_BMP280, data, 2);
    return err;
}

int16_t bmp280_i2c_write_osrs(bmp280_ctrl_meas_t ctrl_meas)
{
    uint8_t reg = REG_CTRL_MEAS;
    uint8_t cfg, data[2];
    int16_t err = bmp280_i2c_read_ctrl_meas(&cfg);
    data[0] = reg;
    data[1] = (cfg & 0x03) | (ctrl_meas.osrs_press << 2) | (ctrl_meas.osrs_tmp << 5);
    err += bmp280_i2c_hal_write(I2C_ADDRESS_BMP280, data, 2);
    bmp280_i2c_hal_ms_delay(100);
    return err;
}

int16_t bmp280_i2c_read_status(bmp280_status_t *sts)
{
    uint8_t reg = REG_STATUS;
    uint8_t data;
    int16_t err = bmp280_i2c_hal_read(I2C_ADDRESS_BMP280, &reg, &data, 1);
    sts->measuring = data & (1 << 3);
    sts->im_update = data & (1 << 0);
    return err;
}

int16_t bmp280_i2c_reset()
{
    uint8_t reg = REG_RESET;
    uint8_t data[2];
    data[0] = reg;
    data[1] = RESET_VAL;
    int16_t err = bmp280_i2c_hal_write(I2C_ADDRESS_BMP280, data, 2);
    return err;
}

int16_t bmp280_i2c_read_pressure_r(int32_t *dt)
{
    uint8_t reg = REG_PRESS_READ;
    uint8_t data[3];
    int16_t err = bmp280_i2c_hal_read(I2C_ADDRESS_BMP280, &reg, data, 3);
    *dt = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    return err;
}

int16_t bmp280_i2c_read_temperature_r(int32_t *dt)
{
    uint8_t reg = REG_TEMP_READ;
    uint8_t data[3];
    int16_t err = bmp280_i2c_hal_read(I2C_ADDRESS_BMP280, &reg, data, 3);
    *dt = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    return err;
} 

int16_t bmp280_i2c_read_data(bmp280_data_t *dt)
{
    int32_t t_fine, press_raw, temp_raw;
    int16_t err = bmp280_i2c_read_pressure_r(&press_raw);
    err += bmp280_i2c_read_temperature_r(&temp_raw);

    if (err != BMP280_OK) 
        return err;

    int32_t var1_t, var2_t, t;
    int64_t var1_p, var2_p, p;
    var1_t = ((((temp_raw>>3) - ((int32_t)calib_params.dig_t1 <<1))) * ((int32_t)calib_params.dig_t2)) >> 11;
    var2_t = (((((temp_raw>>4) - ((int32_t)calib_params.dig_t1)) * ((temp_raw>>4) - ((int32_t)calib_params.dig_t1))) >> 12) * ((int32_t)calib_params.dig_t3)) >> 14;
    t_fine = var1_t + var2_t;
    t = (t_fine * 5 + 128) >> 8;
    dt->temperature = t;

    var1_p = ((int64_t)t_fine) - 128000;
    var2_p = var1_p * var1_p * (int64_t)calib_params.dig_p6;
    var2_p = var2_p + ((var1_p*(int64_t)calib_params.dig_p5)<<17);
    var2_p = var2_p + (((int64_t)calib_params.dig_p4)<<35);
    var1_p = ((var1_p * var1_p * (int64_t)calib_params.dig_p3)>>8) + ((var1_p * (int64_t)calib_params.dig_p2)<<12);
    var1_p = (((((int64_t)1)<<47)+var1_p))*((int64_t)calib_params.dig_p1)>>33;

    if (var1_p == 0) 
        return BMP280_ERR; // avoid exception caused by division by zero

    p = 1048576 - press_raw;
    p = (((p<<31)-var2_p)*3125)/var1_p;
    var1_p = (((int64_t)calib_params.dig_p9) * (p>>13) * (p>>13)) >> 25;
    var2_p = (((int64_t)calib_params.dig_p8) * p) >> 19;
    p = ((p + var1_p + var2_p) >> 8) + (((int64_t)calib_params.dig_p7)<<4);
    dt->pressure = (uint32_t)p;

    return err;
} 

int16_t bmp280_i2c_read_part_number(uint8_t *dt)
{
    uint8_t reg = REG_ID_PARTNUMBER;
    int16_t err = bmp280_i2c_hal_read(I2C_ADDRESS_BMP280, &reg, dt, 1);
    return err;
} 
