#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//BMP280 components
#include "bmp280_i2c.h"
#include "bmp280_i2c_hal.h"

static const char *TAG = "example_usage";

void app_main(void)
{
    esp_err_t err;
    uint8_t id = 0;

    bmp280_i2c_hal_init();

    err = bmp280_i2c_reset();
    if(err != BMP280_OK) ESP_LOGE(TAG, "Error setting the device!");

    err += bmp280_i2c_read_part_number(&id);
    if(err == ESP_OK){
        ESP_LOGI(TAG, "Part number: 0x%02x", id);
    } 
    else{
        ESP_LOGE(TAG, "Unable to read part number!");
    }

    err += bmp280_i2c_set_calib();
    ESP_LOGI(TAG, "Calibration data setting: %s", err == BMP280_OK ? "Successful" : "Failed");

    err += bmp280_i2c_write_power_mode(POWERMODE_NORMAL);
    ESP_LOGI(TAG, "Setting to normal mode: %s", err == BMP280_OK ? "Successful" : "Failed");

    //Config setting. We'll use suggested settings for elevation detection
    err += bmp280_i2c_write_config_filter(FILTER_4);
    bmp280_ctrl_meas_t ctrl_meas = {
        .osrs_press = OSRS_x4,
        .osrs_tmp = OSRS_x1,
    };
    err += bmp280_i2c_write_osrs(ctrl_meas);

    //uint8_t cfg;
    //bmp280_i2c_read_config(&cfg);
    //ESP_LOGW(TAG, "read_config: %d", cfg);

    if (err == BMP280_OK && id == 0x58)
    {
        ESP_LOGI(TAG, "BMP280 initialization successful");
        bmp280_data_t bmp280_dt;
        while(1)
        {
            //Reading here
            if(bmp280_i2c_read_data(&bmp280_dt) == BMP280_OK)
            {
                ESP_LOGI(TAG, "Pressure: %.01f Pa", (float)bmp280_dt.pressure/256);
                ESP_LOGI(TAG, "Temperature: %.01f °C", (float)bmp280_dt.temperature/100);
            }
            else{
                ESP_LOGE(TAG, "Error reading data!");
            }
            
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
    else{
        ESP_LOGE(TAG, "BMP280 initialization failed!");
    }
}
