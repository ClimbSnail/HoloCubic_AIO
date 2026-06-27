#ifndef CLIENT_STUB_H
#define CLIENT_STUB_H

#include "Arduino.h"
#include "Stream.h"
#include "IPAddress.h"

class Client : public Stream
{
public:
    virtual int connect(const char *host, uint16_t port) { return 0; }
    virtual int connect(IPAddress ip, uint16_t port) { return 0; }
    virtual int connect(const char *host, uint16_t port, int timeout) { return 0; }
    virtual size_t write(uint8_t c) override { return 0; }
    virtual size_t write(const uint8_t *buf, size_t size) override { return 0; }
    virtual int available() override { return 0; }
    virtual int read() override { return -1; }
    virtual int read(uint8_t *buf, size_t size) { (void)buf; (void)size; return 0; }
    virtual int peek() override { return -1; }
    virtual void flush() override { }
    virtual void stop() { }
    virtual uint8_t connected() { return 0; }
    virtual int setSocketOption(int, int, int) { return 0; }
};

#endif