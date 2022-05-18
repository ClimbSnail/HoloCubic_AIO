/*
 * start rewrite from:
 * https://github.com/lovyan03/LovyanGFX/blob/master/src/lgfx/v0/platforms/LGFX_PARALLEL_ESP32.hpp
 */
#include "Arduino_DataBus.h"

#if defined(ESP32) && (CONFIG_IDF_TARGET_ESP32S3)

#ifndef _ARDUINO_ESP32LCD16_H_
#define _ARDUINO_ESP32LCD16_H_

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_io_interface.h>
#include <esp_pm.h>
#include <esp_private/gdma.h>
#include <hal/dma_types.h>
#include <hal/lcd_hal.h>
#include <soc/dport_reg.h>
#include <soc/gpio_sig_map.h>
#include <soc/lcd_cam_reg.h>
#include <soc/lcd_cam_struct.h>

#define LCD_MAX_PIXELS_AT_ONCE 2046
#define USE_DMA_THRESHOLD 6

typedef struct esp_lcd_i80_bus_t esp_lcd_i80_bus_t;
typedef struct lcd_panel_io_i80_t lcd_panel_io_i80_t;
typedef struct lcd_i80_trans_descriptor_t lcd_i80_trans_descriptor_t;

struct esp_lcd_i80_bus_t {
    int bus_id;            // Bus ID, index from 0
    portMUX_TYPE spinlock; // spinlock used to protect i80 bus members(hal, device_list, cur_trans)
    lcd_hal_context_t hal; // Hal object
    size_t bus_width;      // Number of data lines
    intr_handle_t intr;    // LCD peripheral interrupt handle
    esp_pm_lock_handle_t pm_lock; // Power management lock
    size_t num_dma_nodes;  // Number of DMA descriptors
    uint8_t *format_buffer;  // The driver allocates an internal buffer for DMA to do data format transformer
    size_t resolution_hz;    // LCD_CLK resolution, determined by selected clock source
    gdma_channel_handle_t dma_chan; // DMA channel handle
    size_t psram_trans_align; // DMA transfer alignment for data allocated from PSRAM
    size_t sram_trans_align;  // DMA transfer alignment for data allocated from SRAM
    lcd_i80_trans_descriptor_t *cur_trans; // Current transaction
    lcd_panel_io_i80_t *cur_device; // Current working device
    LIST_HEAD(i80_device_list, lcd_panel_io_i80_t) device_list; // Head of i80 device list
    struct {
        unsigned int exclusive: 1; // Indicate whether the I80 bus is owned by one device (whose CS GPIO is not assigned) exclusively
    } flags;
    dma_descriptor_t dma_nodes[]; // DMA descriptor pool, the descriptors are shared by all i80 devices
};

struct lcd_i80_trans_descriptor_t {
    lcd_panel_io_i80_t *i80_device; // i80 device issuing this transaction
    int cmd_value;        // Command value
    uint32_t cmd_cycles;  // Command cycles
    const void *data;     // Data buffer
    uint32_t data_length; // Data buffer size
    void *user_ctx;   // private data used by trans_done_cb
    esp_lcd_panel_io_color_trans_done_cb_t trans_done_cb; // transaction done callback
};

struct lcd_panel_io_i80_t {
    esp_lcd_panel_io_t base;   // Base class of generic lcd panel io
    esp_lcd_i80_bus_t *bus;    // Which bus the device is attached to
    int cs_gpio_num;           // GPIO used for CS line
    unsigned int pclk_hz;      // PCLK clock frequency
    size_t clock_prescale;     // Prescaler coefficient, determined by user's configured PCLK frequency
    QueueHandle_t trans_queue; // Transaction queue, transactions in this queue are pending for scheduler to dispatch
    QueueHandle_t done_queue;  // Transaction done queue, transactions in this queue are finished but not recycled by the caller
    size_t queue_size;         // Size of transaction queue
    size_t num_trans_inflight; // Number of transactions that are undergoing (the descriptor not recycled yet)
    int lcd_cmd_bits;          // Bit width of LCD command
    int lcd_param_bits;        // Bit width of LCD parameter
    void *user_ctx;            // private data used when transfer color data
    esp_lcd_panel_io_color_trans_done_cb_t on_color_trans_done; // color data trans done callback
    LIST_ENTRY(lcd_panel_io_i80_t) device_list_entry; // Entry of i80 device list
    struct {
        unsigned int dc_idle_level: 1;  // Level of DC line in IDLE phase
        unsigned int dc_cmd_level: 1;   // Level of DC line in CMD phase
        unsigned int dc_dummy_level: 1; // Level of DC line in DUMMY phase
        unsigned int dc_data_level: 1;  // Level of DC line in DATA phase
    } dc_levels;
    struct {
        unsigned int cs_active_high: 1;     // Whether the CS line is active on high level
        unsigned int reverse_color_bits: 1; // Reverse the data bits, D[N:0] -> D[0:N]
        unsigned int swap_color_bytes: 1;   // Swap adjacent two data bytes before sending out
        unsigned int pclk_active_neg: 1;    // The display will write data lines when there's a falling edge on WR line
        unsigned int pclk_idle_low: 1;      // The WR line keeps at low level in IDLE phase
    } flags;
    lcd_i80_trans_descriptor_t trans_pool[]; // Transaction pool
};

class Arduino_ESP32LCD16 : public Arduino_DataBus
{
public:
  Arduino_ESP32LCD16(
      int8_t dc, int8_t cs, int8_t wr, int8_t rd,
      int8_t d0, int8_t d1, int8_t d2, int8_t d3, int8_t d4, int8_t d5, int8_t d6, int8_t d7,
      int8_t d8, int8_t d9, int8_t d10, int8_t d11, int8_t d12, int8_t d13, int8_t d14, int8_t d15); // Constructor

  void begin(int32_t speed = 0, int8_t dataMode = 0) override;
  void beginWrite() override;
  void endWrite() override;
  void writeCommand(uint8_t) override;
  void writeCommand16(uint16_t) override;
  void write(uint8_t) override;
  void write16(uint16_t) override;
  void writeRepeat(uint16_t p, uint32_t len) override;
  void writePixels(uint16_t *data, uint32_t len) override;

  void writeBytes(uint8_t *data, uint32_t len) override;
  void writePattern(uint8_t *data, uint8_t len, uint32_t repeat) override;

  void writeIndexedPixels(uint8_t *data, uint16_t *idx, uint32_t len) override;
  void writeIndexedPixelsDouble(uint8_t *data, uint16_t *idx, uint32_t len) override;

protected:
  INLINE void WRITECOMMAND16(uint16_t c);
  INLINE void WRITE16(uint16_t d);
  INLINE void WRITE32(uint32_t d);
  INLINE void CS_HIGH(void);
  INLINE void CS_LOW(void);

private:
  int8_t _dc, _cs, _wr, _rd;
  int8_t _d0, _d1, _d2, _d3, _d4, _d5, _d6, _d7;
  int8_t _d8, _d9, _d10, _d11, _d12, _d13, _d14, _d15;

  PORTreg_t _csPortSet; ///< PORT register SET
  PORTreg_t _csPortClr; ///< PORT register CLEAR
  uint32_t _csPinMask;  ///< Bitmask

  esp_lcd_i80_bus_handle_t _i80_bus = nullptr;
  dma_descriptor_t *_dmadesc = nullptr;
  gdma_channel_handle_t _dma_chan;
    union
  {
    uint32_t value;
    struct
    {
      uint8_t value16;
      uint8_t value16_2;
    };
    struct
    {
      uint8_t lsb;
      uint8_t msb;
      uint8_t lsb_2;
      uint8_t msb_2;
    };
  } _data32;
  union
  {
    uint8_t _buffer[LCD_MAX_PIXELS_AT_ONCE * 2] = {0};
    uint16_t _buffer16[LCD_MAX_PIXELS_AT_ONCE];
    uint32_t _buffer32[LCD_MAX_PIXELS_AT_ONCE / 2];
  };
};

#endif // _ARDUINO_ESP32LCD16_H_

#endif // #if defined(ESP32) && (CONFIG_IDF_TARGET_ESP32S3)
