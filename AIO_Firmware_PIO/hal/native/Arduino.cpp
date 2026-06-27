#include "Arduino.h"
#include <chrono>
#include <thread>
#include <FreeRTOS.h>
#include <task.h>

static auto g_start_time = std::chrono::steady_clock::now();

HardwareSerial Serial;

unsigned long millis(void)
{
    auto now = std::chrono::steady_clock::now();
    return (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(now - g_start_time).count();
}

unsigned long micros(void)
{
    auto now = std::chrono::steady_clock::now();
    return (unsigned long)std::chrono::duration_cast<std::chrono::microseconds>(now - g_start_time).count();
}

void delay(unsigned long ms)
{
    if (ms == 0)
    {
        taskYIELD();
    }
    else
    {
        vTaskDelay(pdMS_TO_TICKS(ms));
    }
}

void delayMicroseconds(unsigned int us)
{
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

void yield(void)
{
    taskYIELD();
}

void pinMode(uint8_t pin, uint8_t mode)
{
    (void)pin;
    (void)mode;
}

void digitalWrite(uint8_t pin, uint8_t val)
{
    (void)pin;
    (void)val;
}

int digitalRead(uint8_t pin)
{
    (void)pin;
    return 0;
}

int analogRead(uint8_t pin)
{
    (void)pin;
    return 0;
}

unsigned long getCpuFrequencyMhz(void)
{
    return 240;
}

void setCpuFrequencyMhz(unsigned long freq)
{
    (void)freq;
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}