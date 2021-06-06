/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 * https://github.com/lcdwiki/LCDWIKI_SPI
 */
#include "Arduino_ILI9225.h"

Arduino_ILI9225::Arduino_ILI9225(Arduino_DataBus *bus, int8_t rst, uint8_t r)
    : Arduino_TFT(bus, rst, r, false, ILI9225_TFTWIDTH, ILI9225_TFTHEIGHT, 0, 0, 0, 0)
{
}

void Arduino_ILI9225::begin(int32_t speed)
{
  Arduino_TFT::begin(speed);
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_ILI9225::setRotation(uint8_t r)
{
  Arduino_TFT::setRotation(r);
  _bus->beginWrite();
  switch (_rotation)
  {
  case 3:
    _bus->writeC8D16(ILI9225_DRIVER_OUTPUT_CTRL, 0x031C);
    _bus->writeC8D16(ILI9225_ENTRY_MODE, 0x1038);
    break;
  case 2:
    _bus->writeC8D16(ILI9225_DRIVER_OUTPUT_CTRL, 0x021C);
    _bus->writeC8D16(ILI9225_ENTRY_MODE, 0x1030);
    break;
  case 1:
    _bus->writeC8D16(ILI9225_DRIVER_OUTPUT_CTRL, 0x001C);
    _bus->writeC8D16(ILI9225_ENTRY_MODE, 0x1038);
    break;
  default: // case 0:
    _bus->writeC8D16(ILI9225_DRIVER_OUTPUT_CTRL, 0x011C);
    _bus->writeC8D16(ILI9225_ENTRY_MODE, 0x1030);
    break;
  }
  _bus->endWrite();
}

void Arduino_ILI9225::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  uint8_t cmd1, cmd2, cmd3;

  if ((x != _currentX) || (w != _currentW))
  {
    _currentX = x;
    _currentW = w;
    x += _xStart;
    if (_rotation & 0x01) // Landscape
    {
      cmd1 = ILI9225_VERTICAL_WINDOW_ADDR2;
      cmd2 = ILI9225_VERTICAL_WINDOW_ADDR1;
      cmd3 = ILI9225_RAM_ADDR_SET2;
    }
    else
    {
      cmd1 = ILI9225_HORIZONTAL_WINDOW_ADDR2;
      cmd2 = ILI9225_HORIZONTAL_WINDOW_ADDR1;
      cmd3 = ILI9225_RAM_ADDR_SET1;
    }
    _bus->writeC8D16(cmd1, x);
    _bus->writeC8D16(cmd2, x + w - 1);
    _bus->writeC8D16(cmd3, x);
  }
  if ((y != _currentY) || (h != _currentH))
  {
    _currentY = y;
    _currentH = h;
    y += _yStart;
    if (_rotation & 0x01) // Landscape
    {
      cmd1 = ILI9225_HORIZONTAL_WINDOW_ADDR2;
      cmd2 = ILI9225_HORIZONTAL_WINDOW_ADDR1;
      cmd3 = ILI9225_RAM_ADDR_SET1;
    }
    else
    {
      cmd1 = ILI9225_VERTICAL_WINDOW_ADDR2;
      cmd2 = ILI9225_VERTICAL_WINDOW_ADDR1;
      cmd3 = ILI9225_RAM_ADDR_SET2;
    }
    _bus->writeC8D16(cmd1, y);
    _bus->writeC8D16(cmd2, y + h - 1);
    _bus->writeC8D16(cmd3, y);
  }

  _bus->writeCommand(ILI9225_GRAM_DATA_REG); // write to RAM
}

void Arduino_ILI9225::invertDisplay(bool i)
{
  // Not Implemented
  UNUSED(i);
}

void Arduino_ILI9225::displayOn(void)
{
  _bus->sendCommand(ILI9225_POWER_CTRL1);
  _bus->sendData16(0x0800); // Set SAP,DSTB,STB
}

void Arduino_ILI9225::displayOff(void)
{
  _bus->sendCommand(ILI9225_POWER_CTRL1);
  _bus->sendData16(0x0801); // Set SAP,DSTB,STB
}

void Arduino_ILI9225::tftInit()
{
  if (_rst >= 0)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(100);
    digitalWrite(_rst, LOW);
    delay(ILI9225_RST_DELAY);
    digitalWrite(_rst, HIGH);
    delay(ILI9225_RST_DELAY);
  }
  else
  {
    // Software Rest
  }

  uint8_t ili9225_init_operations[] = {
      BEGIN_WRITE,
      WRITE_C8_D16, ILI9225_LCD_AC_DRIVING_CTRL, 0x01, 0x00,
      WRITE_C8_D16, ILI9225_BLANK_PERIOD_CTRL1, 0x08, 0x08, // set BP and FP
      WRITE_C8_D16, ILI9225_FRAME_CYCLE_CTRL, 0x11, 0x00,   // frame cycle
      WRITE_C8_D16, ILI9225_INTERFACE_CTRL, 0x00, 0x00,     // RGB interface setting R0Ch=0x0110 for RGB 18Bit and R0Ch=0111for RGB16Bit
      WRITE_C8_D16, ILI9225_OSC_CTRL, 0x14, 0x01,           // Set frame rate----0801
      WRITE_C8_D16, ILI9225_VCI_RECYCLING, 0x00, 0x00,      // set system interface
      END_WRITE,

      DELAY, 50,

      //*************Power On sequence ****************//
      BEGIN_WRITE,
      WRITE_C8_D16, ILI9225_POWER_CTRL1, 0x08, 0x00, // Set SAP,DSTB,STB----0A00
      WRITE_C8_D16, ILI9225_POWER_CTRL2, 0x1F, 0x3F, // Set APON,PON,AON,VCI1EN,VC----1038
      END_WRITE,

      DELAY, 50,

      BEGIN_WRITE,
      WRITE_C8_D16, ILI9225_POWER_CTRL3, 0x01, 0x21, // Internal reference voltage= Vci;----1121
      WRITE_C8_D16, ILI9225_POWER_CTRL4, 0x00, 0x6F, // Set GVDD----0066
      WRITE_C8_D16, ILI9225_POWER_CTRL5, 0x43, 0x49, // Set VCOMH/VCOML voltage----5F60
      //-------------- Set GRAM area -----------------//
      WRITE_C8_D16, ILI9225_GATE_SCAN_CTRL, 0x00, 0x00,
      WRITE_C8_D16, ILI9225_VERTICAL_SCROLL_CTRL1, 0x00, 0xDB,
      WRITE_C8_D16, ILI9225_VERTICAL_SCROLL_CTRL2, 0x00, 0x00,
      WRITE_C8_D16, ILI9225_VERTICAL_SCROLL_CTRL3, 0x00, 0x00,
      WRITE_C8_D16, ILI9225_PARTIAL_DRIVING_POS1, 0x00, 0xDB,
      WRITE_C8_D16, ILI9225_PARTIAL_DRIVING_POS2, 0x00, 0x00,
      // ----------- Adjust the Gamma Curve ----------//
      WRITE_C8_D16, ILI9225_GAMMA_CTRL1, 0x00, 0x01,  // 0x0400
      WRITE_C8_D16, ILI9225_GAMMA_CTRL2, 0x20, 0x0B,  // 0x060B
      WRITE_C8_D16, ILI9225_GAMMA_CTRL3, 0x00, 0x00,  // 0x0C0A
      WRITE_C8_D16, ILI9225_GAMMA_CTRL4, 0x04, 0x04,  // 0x0105
      WRITE_C8_D16, ILI9225_GAMMA_CTRL5, 0x0C, 0x0C,  // 0x0A0C
      WRITE_C8_D16, ILI9225_GAMMA_CTRL6, 0x00, 0x0C,  // 0x0B06
      WRITE_C8_D16, ILI9225_GAMMA_CTRL7, 0x01, 0x01,  // 0x0004
      WRITE_C8_D16, ILI9225_GAMMA_CTRL8, 0x04, 0x00,  // 0x0501
      WRITE_C8_D16, ILI9225_GAMMA_CTRL9, 0x11, 0x08,  // 0x0E00
      WRITE_C8_D16, ILI9225_GAMMA_CTRL10, 0x05, 0x0C, // 0x000E
      END_WRITE,

      DELAY, 50,

      BEGIN_WRITE,
      WRITE_C8_D16, ILI9225_DISP_CTRL1, 0x10, 0x17,
      END_WRITE};

  _bus->batchOperation(ili9225_init_operations, sizeof(ili9225_init_operations));
}
