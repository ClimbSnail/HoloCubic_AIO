#pragma once
#ifndef ESP32DMASPI_MASTER_H
#define ESP32DMASPI_MASTER_H

#include <Arduino.h>
#include <SPI.h>
#include <driver/spi_master.h>
#include <deque>

namespace arduino {
namespace esp32 {
namespace spi {
namespace dma {

class Master
{
    spi_device_interface_config_t if_cfg;
    spi_bus_config_t bus_cfg;
    spi_device_handle_t handle;

    spi_host_device_t host { HSPI_HOST };
    uint8_t mode {SPI_MODE3};
    int dma_chan {1}; // must be 1 or 2
    int max_size {4094}; // default size
    uint32_t frequency {SPI_MASTER_FREQ_8M};

    std::deque<spi_transaction_t> transactions;
    int queue_size {1};

public:

    bool begin(uint8_t spi_bus = HSPI, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, int8_t ss = -1);
    bool end();

    uint8_t* allocDMABuffer(size_t s);

    // execute transaction and wait for transmission one by one
    size_t transfer(uint8_t* tx_buf, size_t size);
    size_t transfer(uint8_t* tx_buf, uint8_t* rx_buf, size_t size);

    // queueing transaction and execute simultaneously
    // wait (blocking) and timeout occurs if queue is full with transaction
    // (but designed not to queue transaction more than queue_size, so there is no timeout argument)
    bool queue(uint8_t* tx_buf, size_t size);
    bool queue(uint8_t* tx_buf, uint8_t* rx_buf, size_t size);
    void yield();

    // set these optional parameters before begin() if you want
    void setDataMode(uint8_t m);
    void setFrequency(uint32_t f);
    void setMaxTransferSize(int s);
    void setDMAChannel(int c);
    void setQueueSize(int s);

private:

    void addTransaction(uint8_t* tx_buf, uint8_t* rx_buf, size_t size);

};

} // dma
} // spi
} // esp32
} // arduino

#ifndef ARDUINO_ESP32_DMA_SPI_NAMESPACE_BEGIN
#define ARDUINO_ESP32_DMA_SPI_NAMESPACE_BEGIN namespace arduino { namespace esp32 { namespace spi { namespace dma {
#endif
#ifndef ARDUINO_ESP32_DMA_SPI_NAMESPACE_END
#define ARDUINO_ESP32_DMA_SPI_NAMESPACE_END   }}}}
#endif

namespace ESP32DMASPI = arduino::esp32::spi::dma;

#endif // ESP32DMASPI_MASTER_H
