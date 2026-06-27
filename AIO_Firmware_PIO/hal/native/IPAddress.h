#ifndef IPADDRESS_STUB_H
#define IPADDRESS_STUB_H

#include <stdint.h>
#include "Arduino.h"

class IPAddress
{
public:
    IPAddress() : _addr(0) {}
    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
        _addr = ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)c << 8) | (uint32_t)d;
    }
    IPAddress(uint32_t addr) : _addr(addr) {}
    operator uint32_t() const { return _addr; }
    operator const char *() const { return "0.0.0.0"; }
    operator uint8_t *() { return (uint8_t *)&_addr; }
    bool fromString(const char *str) { (void)str; return true; }
    String toString() const { return "0.0.0.0"; }

private:
    uint32_t _addr;
};

#endif