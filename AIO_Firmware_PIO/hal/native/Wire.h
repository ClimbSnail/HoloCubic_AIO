#ifndef WIRE_STUB_H
#define WIRE_STUB_H

#include <stdint.h>

class TwoWire
{
public:
    void begin() {}
    void begin(int sda, int scl) { (void)sda; (void)scl; }
    void setClock(uint32_t clock) { (void)clock; }
    void beginTransmission(uint8_t addr) {}
    uint8_t endTransmission() { return 0; }
    void write(uint8_t data) {}
    void write(const uint8_t *data, size_t len) { (void)data; (void)len; }
    uint8_t requestFrom(uint8_t addr, uint8_t len) { (void)addr; (void)len; return 0; }
    int available() { return 0; }
    int read() { return 0; }
};

extern TwoWire Wire;

#endif