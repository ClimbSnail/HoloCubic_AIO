#include "esp32-hal.h"

EspClass ESP;

void esp_chip_info(esp_chip_info_t *out_info)
{
    if (out_info) {
        out_info->model = ESP_CHIP_ESP32;
        out_info->features = 0;
        out_info->cores = 2;
        out_info->revision = 1;
    }
}