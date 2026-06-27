#ifndef ESP32_HAL_TIMER_STUB_H
#define ESP32_HAL_TIMER_STUB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*hw_timer_callback_t)(void);

uint64_t timerReadMicros(void);
uint64_t timerReadMilis(void);

#ifdef __cplusplus
}
#endif

#endif /* ESP32_HAL_TIMER_STUB_H */