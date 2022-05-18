/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 */
#include "Arduino_ILI9331.h"
#include "SPI.h"

Arduino_ILI9331::Arduino_ILI9331(Arduino_DataBus *bus, int8_t rst, uint8_t r, bool ips)
    : Arduino_TFT(bus, rst, r, ips, ILI9331_TFTWIDTH, ILI9331_TFTHEIGHT, 0, 0, 0, 0)
{
}

void Arduino_ILI9331::begin(int32_t speed)
{
  Arduino_TFT::begin(speed);
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_ILI9331::setRotation(uint8_t r)
{
  uint16_t gsc1, drvoutctl, entry_mode;
  Arduino_TFT::setRotation(r);
  switch (_rotation)
  {
  case 1:
    gsc1 = 0x2700;
    drvoutctl = 0;
    entry_mode = 0x1038;
    break;
  case 2:
    gsc1 = 0xA700;
    drvoutctl = 0;
    entry_mode = 0x1030;
    break;
  case 3:
    gsc1 = 0xA700;
    drvoutctl = 0x100;
    entry_mode = 0x1038;
    break;
  default: // case 0:
    gsc1 = 0x2700;
    drvoutctl = 0x100;
    entry_mode = 0x1030;
    break;
  }
  _bus->beginWrite();
  _bus->writeC16D16(ILI9331_GSC1, gsc1); // Set the direction of scan by the gate driver
  _bus->writeC16D16(ILI9331_DRVOUTCTL, drvoutctl); // Select the shift direction of outputs from the source driver
  _bus->writeC16D16(ILI9331_ENTRY_MODE, entry_mode); // Set GRAM write direction
  _bus->writeCommand16(ILI9331_MW); // Write to GRAM
  _bus->endWrite();
}

void Arduino_ILI9331::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  if ((x != _currentX) || (w != _currentW))
  {
    _currentX = x;
    _currentW = w;
    x += _xStart;
    _bus->writeC16D16(ILI9331_MC, x);
    if (!(x == _currentX && y == _currentY))
    { 
      _bus->writeC16D16(ILI9331_HSA, x);
      _bus->writeC16D16(ILI9331_HSE, x + w - 1);
    }
  }

  if ((y != _currentY) || (h != _currentH))
  {
    _currentY = y;
    _currentH = h;
    y += _yStart;
    _bus->writeC16D16(ILI9331_MP, y);
    if (!(x == _currentX && y == _currentY))
    { 
      _bus->writeC16D16(ILI9331_VSA, y);
      _bus->writeC16D16(ILI9331_VSE, y + h - 1);
    }
  }

  _bus->writeCommand16(ILI9331_MW);
}

void Arduino_ILI9331::invertDisplay(bool i)
{
  _bus->beginWrite();
  _bus->writeC16D16(ILI9331_GSC2, _ips != i);
  _bus->endWrite();
}

void Arduino_ILI9331::displayOn(void)
{
  _bus->beginWrite();
  _bus->writeC16D16(ILI9331_PWCTL1, 0x1690); // Standby mode OFF
  _bus->writeC16D16(ILI9331_WBRICTRL, 0x0024); // Enable backlight
  _bus->writeC16D16(ILI9331_WBRI, 0x00FF); // Set maximum brightness
  _bus->endWrite();
  delay(100);
}

void Arduino_ILI9331::displayOff(void)
{
  _bus->beginWrite();
  _bus->writeC16D16(ILI9331_PWCTL1, 0x1691); // Standby mode OFF
  _bus->writeC16D16(ILI9331_WBRICTRL, 0x0020); // Disable backlight
  _bus->endWrite();
}

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Arduino_ILI9331::tftInit()
{
  if (_rst != GFX_NOT_DEFINED)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(100);
    digitalWrite(_rst, LOW);
    delay(ILI9331_RST_DELAY);
    digitalWrite(_rst, HIGH);
    delay(ILI9331_RST_DELAY);
  }
  
  uint8_t ili9331_init_operations[] = {
      BEGIN_WRITE,
      //************* Start Initial Sequence **********//
      WRITE_C16_D16, 0x00, 0xE7, 0x10, 0x14,  // Not sure about this.
                                              // This register is not in datasheet and display inits without it,
                                              // but it mentioned in official code snippet.
      WRITE_C16_D16, 0x00, 0x01, 0x01, 0x00,  // set SS and SM bit
      WRITE_C16_D16, 0x00, 0x02, 0x02, 0x00,  // set 1 line inversion
      WRITE_C16_D16, 0x00, 0x03, 0x10, 0x30,  // set GRAM write direction and BGR=1.
      WRITE_C16_D16, 0x00, 0x08, 0x02, 0x07,  // set the back porch and front porch
      WRITE_C16_D16, 0x00, 0x09, 0x00, 0x00,  // set non-display area refresh cycle ISC[3:0]
      WRITE_C16_D16, 0x00, 0x0A, 0x00, 0x00,  // FMARK function
      WRITE_C16_D16, 0x00, 0x0C, 0x00, 0x00,  // RGB interface setting
      WRITE_C16_D16, 0x00, 0x0D, 0x00, 0x00,  // Frame marker Position
      WRITE_C16_D16, 0x00, 0x0F, 0x00, 0x00,  // RGB interface polarity
      //*************Power On sequence ****************//
      WRITE_C16_D16, 0x00, 0x10, 0x00, 0x00,  // SAP, BT[3:0], AP, DSTB, SLP, STB
      WRITE_C16_D16, 0x00, 0x11, 0x00, 0x01,  // DC1[2:0], DC0[2:0], VC[2:0]
      WRITE_C16_D16, 0x00, 0x12, 0x00, 0x00,  // VREG1OUT voltage
      WRITE_C16_D16, 0x00, 0x13, 0x00, 0x00,  // VDV[4:0] for VCOM amplitude
      DELAY, 200,  // Dis-charge capacitor power voltage
      WRITE_C16_D16, 0x00, 0x10, 0x16, 0x90,  // SAP=1, BT=6, APE=1, AP=1, DSTB=0, SLP=0, STB=0
      WRITE_C16_D16, 0x00, 0x11, 0x00, 0x01,  // DC1=2, DC0=2, VC=7
      DELAY, 50,   // wait_ms 50ms
      WRITE_C16_D16, 0x00, 0x12, 0x00, 0x0C,  // Internal reference voltage= Vci; VCIRE=1, PON=0, VRH=5
      DELAY, 50,   // wait_ms 50ms
      WRITE_C16_D16, 0x00, 0x13, 0x07, 0x00,  // VDV=28 for VCOM amplitude
      WRITE_C16_D16, 0x00, 0x29, 0x00, 0x05,  // VCM=10 for VCOMH
      WRITE_C16_D16, 0x00, 0x2B, 0x00, 0x0D,  // Set Frame Rate
      DELAY, 50,   // wait_ms 50ms
      WRITE_C16_D16, 0x00, 0x20, 0x00, 0x00,  // GRAM horizontal Address
      WRITE_C16_D16, 0x00, 0x21, 0x00, 0x00,  // GRAM Vertical Address
      // ----------- Adjust the Gamma Curve ----------//
      WRITE_C16_D16, 0x00, 0x30, 0x00, 0x00,
      WRITE_C16_D16, 0x00, 0x31, 0x02, 0x07,
      WRITE_C16_D16, 0x00, 0x32, 0x00, 0x00,
      WRITE_C16_D16, 0x00, 0x35, 0x00, 0x07,
      WRITE_C16_D16, 0x00, 0x36, 0x05, 0x08,
      WRITE_C16_D16, 0x00, 0x37, 0x07, 0x07,
      WRITE_C16_D16, 0x00, 0x38, 0x00, 0x05,
      WRITE_C16_D16, 0x00, 0x39, 0x07, 0x07,
      WRITE_C16_D16, 0x00, 0x3C, 0x02, 0x02,
      WRITE_C16_D16, 0x00, 0x3D, 0x0A, 0x09,
      //------------------ Set GRAM area ---------------//
      WRITE_C16_D16, 0x00, 0x50, 0x00, 0x00,  // Horizontal GRAM Start Address
      WRITE_C16_D16, 0x00, 0x51, 0x00, 0xEF,  // Horizontal GRAM End Address
      WRITE_C16_D16, 0x00, 0x52, 0x00, 0x00,  // Vertical GRAM Start Address
      WRITE_C16_D16, 0x00, 0x53, 0x01, 0x3F,  // Vertical GRAM Start Address
      WRITE_C16_D16, 0x00, 0x60, 0xA7, 0x00,  // Gate Scan Line GS=0 [0xA700]
      WRITE_C16_D16, 0x00, 0x61, 0x00, 0x01,  // NDL,VLE, REV
      WRITE_C16_D16, 0x00, 0x6A, 0x00, 0x00,  // set scrolling line
      //-------------- Partial Display Control ---------//
      WRITE_C16_D16, 0x00, 0x80, 0x00, 0x00,
      WRITE_C16_D16, 0x00, 0x81, 0x00, 0x00,
      WRITE_C16_D16, 0x00, 0x82, 0x00, 0x00,
      WRITE_C16_D16, 0x00, 0x83, 0x00, 0x00,
      WRITE_C16_D16, 0x00, 0x84, 0x00, 0x00,
      WRITE_C16_D16, 0x00, 0x85, 0x00, 0x00,
      //-------------- Panel Control -------------------//
      WRITE_C16_D16, 0x00, 0x90, 0x00, 0x10,
      WRITE_C16_D16, 0x00, 0x92, 0x06, 0x00,
      WRITE_C16_D16, 0x00, 0x07, 0x01, 0x33,
      //-------------- Backlight Control ---------------//
      WRITE_C16_D16, 0x00, 0xB3, 0x00, 0x24,  // Enable backlight
      WRITE_C16_D16, 0x00, 0xB1, 0x00, 0xFF,  // Set maximum brightness
      END_WRITE};

  _bus->batchOperation(ili9331_init_operations, sizeof(ili9331_init_operations));
  if (_ips)
  {
  _bus->beginWrite();
  _bus->writeC16D16(ILI9331_GSC2, _ips); // Invert display
  _bus->endWrite();
  }
}
