#ifndef STREAM_STUB_H
#define STREAM_STUB_H

#include "Print.h"
#include "Arduino.h"

class Stream : public Print
{
public:
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;

    bool find(const char *target);
    bool find(const char *target, size_t length);
    bool find(uint8_t target);
    String readStringUntil(char terminator);
    String readString();
    size_t readBytes(uint8_t *buffer, size_t length);
    size_t readBytesUntil(char terminator, uint8_t *buffer, size_t length);
    void setTimeout(unsigned long timeout) { _timeout = timeout; }
    size_t readBytes(char *buffer, size_t length) { return readBytes((uint8_t *)buffer, length); }
    size_t readBytesUntil(char terminator, char *buffer, size_t length) { return readBytesUntil(terminator, (uint8_t *)buffer, length); }

protected:
    unsigned long _timeout = 1000;
};

#endif