#include <ESP32DMASPIMaster.h>

ESP32DMASPI::Master master;

static const uint32_t BUFFER_SIZE = 8192;
uint8_t* spi_master_tx_buf;
uint8_t* spi_master_rx_buf;

void set_buffer()
{
    for (uint32_t i = 0; i < BUFFER_SIZE; i++)
    {
        spi_master_tx_buf[i] = i & 0xFF;
    }
    memset(spi_master_rx_buf, 0, BUFFER_SIZE);
}

void setup()
{
    Serial.begin(115200);

    // to use DMA buffer, use these methods to allocate buffer
    spi_master_tx_buf = master.allocDMABuffer(BUFFER_SIZE);
    spi_master_rx_buf = master.allocDMABuffer(BUFFER_SIZE);

    set_buffer();

    delay(5000);

    master.setDataMode(SPI_MODE3);
    // master.setFrequency(SPI_MASTER_FREQ_8M); // too fast for bread board...
    master.setFrequency(4000000);
    master.setMaxTransferSize(BUFFER_SIZE);
    master.setDMAChannel(1); // 1 or 2 only
    master.setQueueSize(1); // transaction queue size
    // begin() after setting
    // HSPI = CS: 15, CLK: 14, MOSI: 13, MISO: 12
    master.begin(); // default SPI is HSPI
}

void loop()
{
    // start and wait to complete transaction
    master.transfer(spi_master_tx_buf, spi_master_rx_buf, BUFFER_SIZE);

    // show received data (if needed)
    for (size_t i = 0; i < BUFFER_SIZE; ++i)
        printf("%d ", spi_master_rx_buf[i]);
    printf("\n");

    delay(2000);
}
