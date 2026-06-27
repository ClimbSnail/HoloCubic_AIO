#ifndef ESP32_HAL_STUB_H
#define ESP32_HAL_STUB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ESP_CHIP_ESP32 = 1,
    ESP_CHIP_ESP32S2 = 2,
    ESP_CHIP_ESP32S3 = 3,
    ESP_CHIP_ESP32C3 = 5,
} esp_chip_model_t;

typedef struct {
    esp_chip_model_t model;
    uint32_t features;
    uint8_t cores;
    uint8_t revision;
} esp_chip_info_t;

void esp_chip_info(esp_chip_info_t *out_info);

#ifdef __cplusplus
}

class EspClass
{
public:
    uint64_t getEfuseMac(void) { return 0x123456789ABCULL; }
    uint32_t getFlashChipSize(void) { return 16 * 1024 * 1024; }
    uint32_t getFreeHeap(void) { return 1024 * 1024; }
    uint32_t getMinFreeHeap(void) { return 512 * 1024; }
    uint32_t getHeapSize(void) { return 4 * 1024 * 1024; }
};

extern EspClass ESP;

#define MALLOC_CAP_DMA  (1 << 1)

#ifdef __cplusplus
extern "C" {
#endif

void *heap_caps_malloc(size_t size, uint32_t caps);

#ifdef __cplusplus
}
#endif

#endif /* __cplusplus */

#endif /* ESP32_HAL_STUB_H */