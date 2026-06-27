#ifndef SPI_STUB_H
#define SPI_STUB_H

#include <stdint.h>

#define SPI_MODE0 0
#define SPI_MODE1 1
#define SPI_MODE2 2
#define SPI_MODE3 3

class SPISettings
{
public:
    SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode)
        : _clock(clock), _bitOrder(bitOrder), _dataMode(dataMode) {}
    SPISettings() : _clock(0), _bitOrder(0), _dataMode(0) {}

private:
    uint32_t _clock;
    uint8_t _bitOrder;
    uint8_t _dataMode;

    friend class SPIClass;
};

class SPIClass
{
public:
    void begin() {}
    void beginTransaction(SPISettings settings) {}
    void endTransaction() {}
    uint8_t transfer(uint8_t data) { return 0; }
    uint16_t transfer16(uint16_t data) { return 0; }
    void transfer(const uint8_t *buf, size_t count) {}
    void transfer(uint8_t *buf, size_t count) {}

    void setFrequency(uint32_t freq) {}
    void setDataMode(uint8_t mode) {}
    void setBitOrder(uint8_t order) {}
};

extern SPIClass SPI;

#endif