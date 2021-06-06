/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 * https://github.com/daumemo/IPS_LCD_R61529_FT6236_Arduino_eSPI_Test
 */
#include "Arduino_R61529.h"

Arduino_R61529::Arduino_R61529(Arduino_DataBus *bus, int8_t rst, uint8_t r, bool ips)
    : Arduino_TFT(bus, rst, r, ips, R61529_TFTWIDTH, R61529_TFTHEIGHT, 0, 0, 0, 0)
{
}

void Arduino_R61529::begin(int32_t speed)
{
  Arduino_TFT::begin(speed);
}

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Arduino_R61529::tftInit()
{
  if (_rst >= 0)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(100);
    digitalWrite(_rst, LOW);
    delay(R61529_RST_DELAY);
    digitalWrite(_rst, HIGH);
    delay(R61529_RST_DELAY);
  }
  else
  {
    // Software Rest
    _bus->sendCommand(R61529_SWRESET);
    delay(R61529_RST_DELAY);
  }

  uint8_t r61529_init_operations[] = {
      BEGIN_WRITE,
      WRITE_COMMAND_8, R61529_SLPOUT,
      END_WRITE,

      DELAY, R61529_SLPOUT_DELAY,

      BEGIN_WRITE,
      WRITE_C8_D8, R61529_MCAP, 0x04,
      WRITE_COMMAND_8, R61529_BLCTL1, //lcd pwm
      WRITE_BYTES, 20,
      0x02, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,

      WRITE_COMMAND_8, R61529_BLCTL2, //lcd pwm
      WRITE_BYTES, 4,
      0x01, // PWMON = 1;
      0x00, // BDCV = 255;
      0xff, // pwm freq = 13.7 kHz
      0x18, // PWMWM = 1; LEDPWME = 1;

      //additional commands:
      WRITE_COMMAND_8, R61529_FMAIS, //Frame Memory Access and Interface Setting
      WRITE_BYTES, 5,
      0x02, // reset start position of a window area address...
      0x00, //TE pin is used. TE signal is output every frame.
      0x00, // empty according to the datasheet - does nothing;
      0x00, // convert 16/18 bits to 24bits data by writing zeroes to LSBs. Sets image data write/read format(?)
      0x00, // ???? (not needed?)
      END_WRITE,

      DELAY, 2,

      BEGIN_WRITE,
      WRITE_C8_D8, R61529_DISPMODE, 0x00, //Display Mode, Uses internal oscillator

      WRITE_COMMAND_8, R61529_PDS, // Panel Driving Setting;
      WRITE_BYTES, 8,
      0x03, // Output polarity is inverted. Left/right interchanging scan. Forward scan. BGR mode (depends on other settings). S960 → S1 (depends)
      0xDF, // Number of lines for driver to drive - 480.
      0x40, // Scan start position - Gate1. (depend on other param);
      0x10, // Dot inversion. Dot inversion in not-lit display area. If 0x13 - both will be set to 'column inversion'.
      0x00, // settings for non-lit display area...
      0x01, // 3 frame scan interval in non-display area...
      0x00, // Source output level in retrace period...
      0x55, //54 . Internal clock divider = 5 (low and high periods).

      WRITE_COMMAND_8, R61529_DTSFNM, //Display Timing Setting for Normal Mode
      WRITE_BYTES, 5,
      0x07, // Clock devider = 12. 14MHz/12. Used by display circuit and step-up circuit.
      0x27, // These bits set the number of clocks in 1 line period. 0x27 - 39 clocks.
      0x08, // Number of back porch lines. 0x08 - 8 lines.
      0x08, // Number of front porch lines. 0x08 - 8lines.
      0x00, // Spacial configuriation mode 1 (?). 1 line inversion mode (?).

      WRITE_COMMAND_8, R61529_SGDTS, // Source/Gate Driving Timing Setting
      WRITE_BYTES, 4,
      0x57, // falling position (stop) of gate driver - 4 clocks... gate start position - 8 clocks...
      0x00, // nothing to set up according to the datasheet
      0x05, // Source precharge period (GND) - 5 clocks.
      0x03, // source precharge period (VCI) - 3 clocks.

      WRITE_C8_D8, R61529_DPIPCTL, 0x04, //DPI polarity control, VSYNC -Active Low. HSYNC - Active Low. DE pin enable data write in when DE=1. Reads data on the rising edge of the PCLK signal.

      //----Gamma setting start-----
      WRITE_COMMAND_8, R61529_GAMMASETA,
      WRITE_BYTES, 24,
      0x03, 0x12, 0x1A, 0x24,
      0x32, 0x4B, 0x3B, 0x29,
      0x1F, 0x18, 0x12, 0x04,
      0x03, 0x12, 0x1A, 0x24,
      0x32, 0x4B, 0x3B, 0x29,
      0x1F, 0x18, 0x12, 0x04,

      WRITE_COMMAND_8, R61529_GAMMASETB,
      WRITE_BYTES, 24,
      0x03, 0x12, 0x1A, 0x24,
      0x32, 0x4B, 0x3B, 0x29,
      0x1F, 0x18, 0x12, 0x04,
      0x03, 0x12, 0x1A, 0x24,
      0x32, 0x4B, 0x3B, 0x29,
      0x1F, 0x18, 0x12, 0x04,

      WRITE_COMMAND_8, R61529_GAMMASETC,
      WRITE_BYTES, 24,
      0x03, 0x12, 0x1A, 0x24,
      0x32, 0x4B, 0x3B, 0x29,
      0x1F, 0x18, 0x12, 0x04,
      0x03, 0x12, 0x1A, 0x24,
      0x32, 0x4B, 0x3B, 0x29,
      0x1F, 0x18, 0x12, 0x04,
      //---Gamma setting end--------

      //old ones:
      WRITE_COMMAND_8, R61529_PWSET,
      WRITE_BYTES, 16,
      0x99, //DC4~1//A5. Set up clock cycle of the internal step up controller.
      0x06, //BT // Set Voltage step up factor.
      0x08, // default according to the datasheet - does nothing.
      0x20, // VCN step up cycles.
      0x29, //VC1, VC2// VCI3 voltage = 2.70V;  VCI2 voltage = 3.8V.
      0x04, // default
      0x01, // default
      0x00, // default
      0x08, // default
      0x01, // default
      0x00, // default
      0x06, // default
      0x01, // default
      0x00, // default
      0x00, // default
      0x20, // default

      WRITE_COMMAND_8, R61529_VCOMSET, //VCOM setting
      WRITE_BYTES, 4,
      0x00, //disable write to VDC[7:0]
      0x20, //45 38 VPLVL// voltage of γ correction registers for positive polarity
      0x20, //45 38 VNLVL// voltage of γ correction registers for negative polarity
      0x15, //32 2A VCOMDC// VNLVL x 0.063

      WRITE_COMMAND_8, R61529_NVMAC, //NVM Access Control
      WRITE_BYTES, 3,
      0x00, //NVM access is disabled
      0x00, //Erase operation (disabled).
      0x00, //TE pin works as tearing effect pin.
            // should be one more writeData(0x00}, according to the datasheet.

      WRITE_COMMAND_8, R61529_SETDDBWRCTL, //set_DDB_write_control
      WRITE_BYTES, 6,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00,

      WRITE_C8_D8, R61529_NVMLC, 0x00, //NVM Load Control, does not execute data load from the NVM to each command

      WRITE_C8_D8, 0x3A, 0x55, // set_pixel_format, 16-Bit/pixel = 55h, 24-bit/pixel = 77h

      WRITE_COMMAND_8, R61529_DISPON,
      END_WRITE,

      DELAY, 20};

  _bus->batchOperation(r61529_init_operations, sizeof(r61529_init_operations));
}

void Arduino_R61529::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  if ((x != _currentX) || (w != _currentW))
  {
    _bus->writeC8D16D16(R61529_CASET, x + _xStart, x + w - 1 + _xStart);

    _currentX = x;
    _currentW = w;
  }
  if ((y != _currentY) || (h != _currentH))
  {
    _bus->writeC8D16D16(R61529_PASET, y + _yStart, y + h - 1 + _yStart);

    _currentY = y;
    _currentH = h;
  }

  _bus->writeCommand(R61529_RAMWR); // write to RAM
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_R61529::setRotation(uint8_t r)
{
  Arduino_TFT::setRotation(r);
  switch (_rotation)
  {
  case 0:
    r = R61529_MADCTL_RGB;
    break;
  case 1:
    r = R61529_MADCTL_MV | R61529_MADCTL_MX | R61529_MADCTL_RGB;
    break;
  case 2:
    r = R61529_MADCTL_RGB | R61529_MADCTL_GS | R61529_MADCTL_MX;
    break;
  case 3:
    r = R61529_MADCTL_MV | R61529_MADCTL_RGB | R61529_MADCTL_GS;
    break;
  }

  _bus->beginWrite();
  _bus->writeCommand(R61529_MADCTL);
  _bus->write(r);
  _bus->endWrite();
}

void Arduino_R61529::invertDisplay(bool i)
{
  _bus->sendCommand(i ? R61529_INVON : R61529_INVOFF);
}

void Arduino_R61529::displayOn(void)
{
  _bus->sendCommand(R61529_SLPOUT);
  delay(R61529_SLPOUT_DELAY);
}

void Arduino_R61529::displayOff(void)
{
  _bus->sendCommand(R61529_SLPIN);
  delay(R61529_SLPIN_DELAY);
}
