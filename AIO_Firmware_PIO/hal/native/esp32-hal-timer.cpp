#include "esp32-hal-timer.h"
#include <chrono>

static auto g_timer_start = std::chrono::steady_clock::now();

uint64_t timerReadMicros(void)
{
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - g_timer_start).count();
}

uint64_t timerReadMilis(void)
{
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - g_timer_start).count();
}