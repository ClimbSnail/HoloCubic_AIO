#ifndef ESP_SYSTEM_STUB_H
#define ESP_SYSTEM_STUB_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void esp_restart(void);
uint32_t esp_random(void);
uint32_t esp_get_free_heap_size(void);
uint32_t esp_get_minimum_free_heap_size(void);

#ifdef __cplusplus
}
#endif

#endif