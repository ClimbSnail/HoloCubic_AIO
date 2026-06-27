#ifndef ARDUINO_STUB_H
#define ARDUINO_STUB_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <semphr.h>
#include "esp32-hal.h"
#include "log_buffer.h"
#include <ctype.h>

typedef bool boolean;
typedef uint8_t byte;
typedef uint16_t word;

#define HIGH 0x1
#define LOW  0x0

#define FILE_READ  "rb"
#define FILE_WRITE "wb"
#define FILE_APPEND "a"

#define memcpy_P memcpy
#define memcmp_P memcmp

#define INPUT        0x01
#define OUTPUT       0x02
#define INPUT_PULLUP 0x05

#define A0 36
#define A1 37
#define A2 38
#define A3 39

#define PROGMEM
#define PGM_P const char *
#define F(string_literal) (string_literal)
#define pgm_read_byte_near(addr) (*((const uint8_t *)(addr)))
#define pgm_read_byte(addr)       (*((const uint8_t *)(addr)))
#define pgm_read_word(addr)       (*((const uint16_t *)(addr)))

#define PI 3.14159265358979323846

#ifdef __cplusplus
#include <string>

class __FlashStringHelper;
typedef const __FlashStringHelper *FlashString;
#endif

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#define LOW  0x0

#ifdef __cplusplus
#include <algorithm>
#include <cmath>
using std::min;
using std::max;
#else
#define min(a, b)                 ((a) < (b) ? (a) : (b))
#define max(a, b)                 ((a) > (b) ? (a) : (b))
#define abs(x)                    ((x) > 0 ? (x) : -(x))
#define round(x)                  ((x) >= 0 ? (long)((x) + 0.5) : (long)((x) - 0.5))
#endif
#define radians(deg)              ((deg) * PI / 180.0)
#define degrees(rad)              ((rad) * 180.0 / PI)

#ifdef __cplusplus

class Print;

class Printable
{
public:
    virtual size_t printTo(Print& p) const = 0;
};

class String : public Printable
{
public:
    String() : _str() {}
    String(const char *cstr) : _str(cstr ? cstr : "") {}
    String(const char *cstr, int len) : _str(cstr ? std::string(cstr, (size_t)len) : "") {}
    String(const String &s) : _str(s._str) {}
    String(const std::string &s) : _str(s) {}
    String(int val) { char buf[32]; snprintf(buf, sizeof(buf), "%d", val); _str = buf; }
    String(unsigned int val) { char buf[32]; snprintf(buf, sizeof(buf), "%u", val); _str = buf; }
    String(long val) { char buf[32]; snprintf(buf, sizeof(buf), "%ld", val); _str = buf; }
    String(unsigned long val) { char buf[32]; snprintf(buf, sizeof(buf), "%lu", val); _str = buf; }
    String(long long val) { char buf[32]; snprintf(buf, sizeof(buf), "%lld", val); _str = buf; }
    String(unsigned long long val) { char buf[32]; snprintf(buf, sizeof(buf), "%llu", val); _str = buf; }
    String(float val) { char buf[32]; snprintf(buf, sizeof(buf), "%f", val); _str = buf; }
    String(double val) { char buf[32]; snprintf(buf, sizeof(buf), "%f", val); _str = buf; }
    String(double val, int decimals) { char fmt[16]; snprintf(fmt, sizeof(fmt), "%%.%df", decimals); char buf[64]; snprintf(buf, sizeof(buf), fmt, val); _str = buf; }

    const char *c_str() const { return _str.c_str(); }
    size_t length() const { return _str.length(); }
    int toInt() const { return atoi(_str.c_str()); }
    float toFloat() const { return (float)atof(_str.c_str()); }

    std::string::iterator begin() { return _str.begin(); }
    std::string::const_iterator begin() const { return _str.begin(); }
    std::string::iterator end() { return _str.end(); }
    std::string::const_iterator end() const { return _str.end(); }

    String &operator=(const char *cstr) { _str = cstr ? cstr : ""; return *this; }
    String &operator=(const String &s) { _str = s._str; return *this; }
    String &operator+=(const char *cstr) { _str += cstr; return *this; }
    String &operator+=(const String &s) { _str += s._str; return *this; }
    String &operator+=(char c) { _str += c; return *this; }
    String &operator+=(int val) { char buf[32]; snprintf(buf, sizeof(buf), "%d", val); _str += buf; return *this; }

    bool concat(const char *cstr) { if (cstr) _str += cstr; return true; }
    bool concat(const String &s) { _str += s._str; return true; }
    bool concat(char c) { _str += c; return true; }
    bool concat(int val) { char buf[32]; snprintf(buf, sizeof(buf), "%d", val); _str += buf; return true; }

    String operator+(const char *cstr) const { String r(*this); r += cstr; return r; }
    String operator+(const String &s) const { String r(*this); r += s._str; return r; }
    String operator+(int val) const { String r(*this); r += val; return r; }

    bool operator==(const char *cstr) const { return _str == (cstr ? cstr : ""); }
    bool operator==(const String &s) const { return _str == s._str; }
    bool operator!=(const char *cstr) const { return !(*this == cstr); }
    bool operator!=(const String &s) const { return !(*this == s); }
    bool operator<(const String &s) const { return _str < s._str; }
    bool operator>(const String &s) const { return _str > s._str; }

    char operator[](int index) const { return _str[index]; }
    char &operator[](int index) { return _str[index]; }
    char charAt(int index) const { return _str[index]; }

    bool startsWith(const String &prefix) const { return _str.rfind(prefix._str, 0) == 0; }
    bool endsWith(const String &suffix) const {
        if (_str.length() >= suffix._str.length())
            return _str.compare(_str.length() - suffix._str.length(), suffix._str.length(), suffix._str) == 0;
        return false;
    }
    int indexOf(char c) const {
        size_t pos = _str.find(c);
        return (pos != std::string::npos) ? (int)pos : -1;
    }
    int indexOf(const char *str) const {
        size_t pos = _str.find(str ? str : "");
        return (pos != std::string::npos) ? (int)pos : -1;
    }
    int indexOf(const String &str) const {
        size_t pos = _str.find(str._str);
        return (pos != std::string::npos) ? (int)pos : -1;
    }
    int indexOf(char c, int from) const {
        size_t pos = _str.find(c, (size_t)from);
        return (pos != std::string::npos) ? (int)pos : -1;
    }
    int indexOf(const char *str, int from) const {
        size_t pos = _str.find(str ? str : "", (size_t)from);
        return (pos != std::string::npos) ? (int)pos : -1;
    }
    int lastIndexOf(char c) const {
        size_t pos = _str.rfind(c);
        return (pos != std::string::npos) ? (int)pos : -1;
    }
    String substring(int start, int len = -1) const {
        if (len < 0) return String(_str.substr(start));
        return String(_str.substr(start, len));
    }
    void remove(int index, int count) { _str.erase(index, count); }
    void toLowerCase() { for (auto &c : _str) c = tolower(c); }
    void toUpperCase() { for (auto &c : _str) c = toupper(c); }
    void trim() {
        size_t start = _str.find_first_not_of(" \t\n\r");
        size_t end = _str.find_last_not_of(" \t\n\r");
        if (start == std::string::npos) _str.clear();
        else _str = _str.substr(start, end - start + 1);
    }

    size_t printTo(Print& p) const;

private:
    std::string _str;
};

inline String operator+(const char *lhs, const String &rhs) { return String(lhs) + rhs; }

class StringSumHelper : public String {};

class Print
{
public:
    virtual size_t write(uint8_t) = 0;
    virtual size_t write(const uint8_t *buffer, size_t size) {
        size_t n = 0;
        while (size--) { write(*buffer++); n++; }
        return n;
    }
    virtual void flush() {}
    size_t write(const char *str) { return write((const uint8_t *)str, strlen(str)); }

    size_t print(const char *s) { return write((const uint8_t *)s, strlen(s)); }
    size_t print(const String &s) { return write((const uint8_t *)s.c_str(), s.length()); }
    size_t print(int n) { char buf[32]; snprintf(buf, sizeof(buf), "%d", n); return print(buf); }
    size_t print(unsigned int n) { char buf[32]; snprintf(buf, sizeof(buf), "%u", n); return print(buf); }
    size_t print(long n) { char buf[32]; snprintf(buf, sizeof(buf), "%ld", n); return print(buf); }
    size_t print(unsigned long n) { char buf[32]; snprintf(buf, sizeof(buf), "%lu", n); return print(buf); }
    size_t print(long long n) { char buf[32]; snprintf(buf, sizeof(buf), "%lld", n); return print(buf); }
    size_t print(unsigned long long n) { char buf[32]; snprintf(buf, sizeof(buf), "%llu", n); return print(buf); }
    size_t print(float n, int digits = 2) { char buf[32]; snprintf(buf, sizeof(buf), "%.*f", digits, n); return print(buf); }
    size_t print(double n, int digits = 2) { char buf[32]; snprintf(buf, sizeof(buf), "%.*f", digits, n); return print(buf); }

    size_t println(void) { return print("\n"); }
    size_t println(const char *s) { size_t n = print(s); return n + println(); }
    size_t println(const String &s) { size_t n = print(s); return n + println(); }
    size_t println(int n) { size_t s = print(n); return s + println(); }
    size_t println(unsigned int n) { size_t s = print(n); return s + println(); }
    size_t println(long n) { size_t s = print(n); return s + println(); }
    size_t println(unsigned long n) { size_t s = print(n); return s + println(); }
    size_t println(long long n) { size_t s = print(n); return s + println(); }
    size_t println(unsigned long long n) { size_t s = print(n); return s + println(); }
    size_t println(float n, int digits = 2) { size_t s = print(n, digits); return s + println(); }
    size_t println(double n, int digits = 2) { size_t s = print(n, digits); return s + println(); }

    size_t printf(const char *fmt, ...);
    size_t printf_loc(const char *file, int line, const char *fmt, ...);
};

inline size_t String::printTo(Print& p) const { return p.write((const uint8_t*)_str.c_str(), _str.length()); }

class HardwareSerial : public Print
{
public:
    void begin(unsigned long baud) { (void)baud; }
    void flush() { log_buffer_flush(); }
    size_t write(uint8_t c) override {
        log_buffer_write((const char *)&c, 1);
        return 1;
    }
    size_t write(const uint8_t *buffer, size_t size) override {
        if (buffer && size > 0) {
            log_buffer_write((const char *)buffer, (uint32_t)size);
        }
        return size;
    }
    size_t write(const char *buffer, size_t size) {
        return write((const uint8_t *)buffer, size);
    }
    int available() { return 0; }
    int read() { return -1; }
    int read(uint8_t *buf, size_t size) { (void)buf; (void)size; return 0; }
    int peek() { return -1; }
};

extern HardwareSerial Serial;

#else /* __cplusplus */

typedef struct { int dummy; } String;
typedef struct { int dummy; } Print;
typedef struct { int dummy; } HardwareSerial;

#endif /* __cplusplus */


#ifdef __cplusplus
extern "C" {
#endif

unsigned long millis(void);
unsigned long micros(void);
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);
void yield(void);
long map(long x, long in_min, long in_max, long out_min, long out_max);
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
int analogRead(uint8_t pin);
unsigned long getCpuFrequencyMhz(void);
void setCpuFrequencyMhz(unsigned long freq);

#ifdef __cplusplus
}

// Print::printf / printf_loc 实现（依赖 millis()，必须在 millis() 声明之后）
inline size_t Print::printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buf[512];
    int prefix_len = snprintf(buf, sizeof(buf), "[%lu] ", (unsigned long)millis());
    vsnprintf(buf + prefix_len, sizeof(buf) - prefix_len, fmt, args);
    va_end(args);
    return print(buf);
}

inline size_t Print::printf_loc(const char *file, int line, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buf[512];
    const char *basename = file;
    for (const char *p = file; *p; p++) {
        if (*p == '/' || *p == '\\') basename = p + 1;
    }
    int prefix_len = snprintf(buf, sizeof(buf), "[%lu][%s:%d] ", (unsigned long)millis(), basename, line);
    vsnprintf(buf + prefix_len, sizeof(buf) - prefix_len, fmt, args);
    va_end(args);
    return print(buf);
}

#define SERIAL_LOGF(fmt, ...) Serial.printf_loc(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

inline long random(long max) { return rand() % max; }
inline long random(long min, long max) { return min + rand() % (max - min); }
inline void randomSeed(unsigned long seed) { srand(seed); }
#endif

#endif /* ARDUINO_STUB_H */