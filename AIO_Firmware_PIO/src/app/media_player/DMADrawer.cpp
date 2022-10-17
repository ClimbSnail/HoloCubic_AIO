/*
 original source is here.
  https://github.com/espressif/esp-idf/blob/master/examples/peripherals/spi_master/main/spi_master_example_main.c

 Modify by lovyan03
*/

#pragma GCC optimize("O3")

#include "DMADrawer.h"

#include <string.h>
#include <esp_heap_caps.h>
#include <driver/spi_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "common.h"

static spi_device_handle_t _spi = NULL;
static spi_transaction_t trans[6];
static gpio_num_t tft_dc_pin, tft_cs_pin;
static uint16_t *_pixBuf[2];
static uint16_t _lastX, _lastY, _lastW, _lastH;
static uint8_t _pixFlip = 0;
static bool _sent[6];

static void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc = (int)t->user;
    gpio_set_level(tft_dc_pin, dc);
    if (-1 != tft_cs_pin)
    {
        gpio_set_level(tft_cs_pin, 0);
    }
}
static void lcd_spi_post_transfer_callback(spi_transaction_t *t)
{
    gpio_set_level(tft_cs_pin, 1);
}

static spi_device_handle_t spi_start(uint32_t len, int spi_freq, int tft_mosi, int tft_miso, int tft_sclk, int tft_cs, int tft_dc)
{
    tft_dc_pin = (gpio_num_t)tft_dc;
    tft_cs_pin = (gpio_num_t)tft_cs;
    esp_err_t ret;
    spi_device_handle_t hSpi;
    spi_bus_config_t buscfg = {
        .mosi_io_num = (gpio_num_t)tft_mosi,
        .miso_io_num = (gpio_num_t)tft_miso,
        .sclk_io_num = (gpio_num_t)tft_sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = len + 8,
        .flags = SPICOMMON_BUSFLAG_MASTER,
        .intr_flags = 0};
    ret = spi_bus_initialize(VSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);

    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0, //SPI mode 0
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = spi_freq,
        .input_delay_ns = 0,
        .spics_io_num = -1,
        .flags = SPI_DEVICE_NO_DUMMY,
        .queue_size = 7,                         //We want to be able to queue 7 transactions at a time
        .pre_cb = lcd_spi_pre_transfer_callback, //Specify pre-transfer callback to handle D/C line
        .post_cb = lcd_spi_post_transfer_callback};
    ret = spi_bus_add_device(VSPI_HOST, &devcfg, &hSpi);
    ESP_ERROR_CHECK(ret);
    return hSpi;
}

bool DMADrawer::setup(uint32_t bufsize, int spi_freq, int tft_mosi, int tft_miso, int tft_sclk, int tft_cs, int tft_dc)
{
    for (int i = 0; i < 2; ++i)
    {
        _pixBuf[i] = NULL;
        // _pixBuf[i] = (uint16_t *)pvPortMallocCaps(bufsize, MALLOC_CAP_DMA);
        _pixBuf[i] = (uint16_t *)heap_caps_malloc(bufsize, MALLOC_CAP_DMA); // 一个字节
        if (NULL == _pixBuf[i])
            Serial.println("Fail _pixBuf[i]");
    }
    _lastX = -1;
    _lastY = -1;
    _lastW = -1;
    _lastH = -1;
    for (int i = 0; i < 6; ++i)
    {
        _sent[i] = false;
        trans[i].flags = SPI_TRANS_USE_TXDATA;
        trans[i].user = (void *)(i & 1);
        trans[i].cmd = trans[i].addr = trans[i].rxlength = 0;
        trans[i].length = (trans[i].user) ? 8 * 4 : 8;
    }
    trans[0].tx_data[0] = 0x2A; // Column address set
    trans[2].tx_data[0] = 0x2B; // Page address set
    trans[4].tx_data[0] = 0x2C; // Memory write
    trans[5].flags = 0;

    if (0 <= tft_dc)
        gpio_set_direction((gpio_num_t)tft_dc, GPIO_MODE_OUTPUT);
    if (_spi == NULL)
    {
        _spi = spi_start(bufsize, spi_freq, tft_mosi, tft_miso, tft_sclk, tft_cs, tft_dc);
    }

    return true;
}

void DMADrawer::close()
{
    waitDraw();
    for (int i = 0; i < 2; ++i)
    {
        free(_pixBuf[i]);
        _pixBuf[i] = NULL;
    }
}

uint16_t *DMADrawer::getNextBuffer()
{
    return _pixBuf[_pixFlip];
}

void DMADrawer::draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    esp_err_t ret;
    spi_transaction_t *tr;
    for (uint8_t i = 0; i < 6; i++)
    {
        Serial.print("for :");
        Serial.println(i);
        if (_sent[i])
        {
            Serial.println("spi_device_get_trans_result");
            ret = spi_device_get_trans_result(_spi, &tr, portMAX_DELAY);
            assert(ret == ESP_OK);
            _sent[i] = false;
        }
        tr = &trans[i];
        switch (i)
        {
        case 0:
            if (_lastX == x && _lastW == w)
                continue;
            break;

        case 1:
            if (_lastX == x && _lastW == w)
                continue;
            tr->tx_data[0] = x >> 8;             //Start Col High
            tr->tx_data[1] = x & 0xFF;           //Start Col Low
            tr->tx_data[2] = (x + w - 1) >> 8;   //End Col High
            tr->tx_data[3] = (x + w - 1) & 0xFF; //End Col Low
            break;

        case 2:
            if (_lastY == y && _lastH == h)
                continue;
            break;

        case 3:
            if (_lastY == y && _lastH == h)
                continue;
            tr->tx_data[0] = y >> 8;             //Start page high
            tr->tx_data[1] = y & 0xFF;           //start page low
            tr->tx_data[2] = (y + h - 1) >> 8;   //end page high
            tr->tx_data[3] = (y + h - 1) & 0xFF; //end page low
            break;

        case 5:
            tr->tx_buffer = _pixBuf[_pixFlip]; //finally send the line data
            tr->length = w * h * 16;           //Data length, in bits
            _pixFlip = !_pixFlip;
            break;

        default:
            break;
        }
        tr->rxlength = 0;
        Serial.println("spi_device_queue_trans");
        ret = spi_device_queue_trans(_spi, tr, portMAX_DELAY);
        assert(ret == ESP_OK);
        _sent[i] = true;
    }
    _lastX = x;
    _lastY = y;
    _lastW = w;
    _lastH = h;
}

void DMADrawer::waitDraw()
{
    spi_transaction_t *rtrans;
    esp_err_t ret;
    for (uint8_t i = 0; i < 6; ++i)
    {
        if (!_sent[i])
            continue;
        ret = spi_device_get_trans_result(_spi, &rtrans, portMAX_DELAY);
        assert(ret == ESP_OK);
        _sent[i] = false;
    }
}
