/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 * https://github.com/daumemo/IPS_LCD_NT39125_FT6236_Arduino_eSPI_Test
 */
#include "Arduino_NT39125.h"

Arduino_NT39125::Arduino_NT39125(
    Arduino_DataBus *bus, int8_t rst, uint8_t r,
    bool ips, int16_t w, int16_t h,
    uint8_t col_offset1, uint8_t row_offset1, uint8_t col_offset2, uint8_t row_offset2)
    : Arduino_TFT(bus, rst, r, ips, w, h, col_offset1, row_offset1, col_offset2, row_offset2)
{
}

void Arduino_NT39125::begin(int32_t speed)
{
  Arduino_TFT::begin(speed);
}

void Arduino_NT39125::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  if ((x != _currentX) || (w != _currentW))
  {
    _bus->writeC8D16D16(NT39125_CASET, x + _xStart, x + w - 1 + _xStart);

    _currentX = x;
    _currentW = w;
  }
  if ((y != _currentY) || (h != _currentH))
  {
    _bus->writeC8D16D16(NT39125_RASET, y + _yStart, y + h - 1 + _yStart);

    _currentY = y;
    _currentH = h;
  }

  _bus->writeCommand(NT39125_RAMWR); // write to RAM
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_NT39125::setRotation(uint8_t r)
{
  Arduino_TFT::setRotation(r);
  switch (_rotation)
  {
  case 3:
    r = NT39125_MADCTL_BGR | NT39125_MADCTL_MV | NT39125_MADCTL_MX | NT39125_MADCTL_MY;
    break;
  case 2:
    r = NT39125_MADCTL_BGR | NT39125_MADCTL_MY;
    break;
  case 1:
    r = NT39125_MADCTL_BGR | NT39125_MADCTL_MV;
    break;
  default: // case 0:
    r = NT39125_MADCTL_BGR | NT39125_MADCTL_MX;
    break;
  }

  _bus->beginWrite();
  _bus->writeCommand(NT39125_MADCTL);
  _bus->write(r);
  _bus->endWrite();
}

void Arduino_NT39125::invertDisplay(bool i)
{
  _bus->sendCommand((_ips ^ i) ? NT39125_INVON : NT39125_INVOFF);
}

void Arduino_NT39125::displayOn(void)
{
  _bus->sendCommand(NT39125_SLPOUT);
  delay(NT39125_SLPOUT_DELAY);
}

void Arduino_NT39125::displayOff(void)
{
  _bus->sendCommand(NT39125_SLPIN);
  delay(NT39125_SLPIN_DELAY);
}

void Arduino_NT39125::tftInit()
{
  if (_rst >= 0)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(100);
    digitalWrite(_rst, LOW);
    delay(NT39125_RST_DELAY);
    digitalWrite(_rst, HIGH);
    delay(NT39125_RST_DELAY);
  }
  else
  {
    // Software Rest
    _bus->sendCommand(NT39125_SWRESET);
    delay(NT39125_RST_DELAY);
  }

  uint8_t nt39125_init_operations[] = {
      //Initializing
      BEGIN_WRITE,
      WRITE_COMMAND_8, NT39125_SLPOUT, // Sleep Out
      END_WRITE,

      DELAY, 250,
      DELAY, 250,
      DELAY, 250,
      DELAY, 250,
      DELAY, 200,

      //Display Settings
      BEGIN_WRITE,
      WRITE_C8_D8, NT39125_TEOFF,                // Tearing Effect Line OFF
      WRITE_C8_D16, NT39125_FRMCTR1, 0x11, 0x1b, // Set Division ratio for internal clocks of Normal mode
      WRITE_C8_D16, NT39125_FRMCTR2, 0x11, 0x1b, // Set Division ratio for internal clocks of Idle mode
      WRITE_C8_D16, NT39125_FRMCTR3, 0x11, 0x1b, // Set Division ratio for internal clocks of Partial mode (Idle mode off)
      WRITE_C8_D8, NT39125_INVCTR, 0x02,         // Inversion Control
      WRITE_C8_D16, NT39125_DISSET5, 0x01, 0x02, // Display Function set 5
      WRITE_C8_D8, NT39125_PWCTR1, 0x24,         // Power Control 1, 4.1V

      // VGL -7V
      WRITE_C8_D16, NT39125_PWCTR2, 0x02, 0x00, // Power Control 2
      // WRITE_C8_D16, NT39125_PWCTR2, 0x02, 0x07, // gate modulation removed (spec 1.03 version)
      //VGL -7V

      WRITE_C8_D16, NT39125_PWCTR3, 0x05, 0x01, // Power Control 3 (in Normal mode/ Full colors), e1 setting
      WRITE_C8_D16, NT39125_PWCTR4, 0x02, 0x05, // Power Control 4 (in Idle mode/ 8-colors)
      WRITE_C8_D16, NT39125_PWCTR5, 0x02, 0x04, // Power Control 5 (in Partial mode/ full-colors)
      WRITE_C8_D16, NT39125_VMCTR1,             // VCOM Control, Chameleon
      0x14,                                     // 3   	.	 VcomH
      0x2e,                                     // -1.35	.VcomL
      WRITE_C8_D8, NT39125_GAM_R_SEL, 0x01,     // Gamma Selection

      ///////////////////////////////////////// gamma //////////////////////

      WRITE_COMMAND_8, NT39125_GMCTRP0, // Positive RED Gamma Control, d1 third vibration
      WRITE_BYTES, 15,
      0x27, 0x2B, 0x2E, 0x06,
      0x0D, 0x11, 0x28, 0x7B,
      0x35, 0x0C, 0x20, 0x26,
      0x25, 0x28, 0x3C,

      WRITE_COMMAND_8, NT39125_GMCTRN0, // Negative RED Gamma Control
      WRITE_BYTES, 15,
      0x08, 0x21, 0x26, 0x09,
      0x0F, 0x12, 0x1F, 0x38,
      0x31, 0x0D, 0x23, 0x29,
      0x2C, 0x2F, 0x33,

      WRITE_COMMAND_8, NT39125_GMCTRP1, // Positive GREEN Gamma Control
      WRITE_BYTES, 15,
      0x27, 0x2C, 0x2F, 0x07,
      0x0E, 0x11, 0x29, 0x7A,
      0x35, 0x0C, 0x20, 0x26,
      0x24, 0x29, 0x3C,

      WRITE_COMMAND_8, NT39125_GMCTRN1, // Negative GREEN Gamma Control
      WRITE_BYTES, 15,
      0x08, 0x20, 0x26, 0x09,
      0x0F, 0x12, 0x1F, 0x48,
      0x30, 0x0D, 0x22, 0x28,
      0x2B, 0x2E, 0x33,

      WRITE_COMMAND_8, NT39125_GMCTRP2, // Positive BLUE Gamma Control
      WRITE_BYTES, 15,
      0x1F, 0x24, 0x27, 0x08,
      0x0F, 0x12, 0x25, 0x7B,
      0x32, 0x0C, 0x20, 0x26,
      0x20, 0x25, 0x3C,

      WRITE_COMMAND_8, NT39125_GMCTRN2, // Negative BLUE Gamma Control
      WRITE_BYTES, 15,
      0x08, 0x24, 0x2B, 0x09,
      0x0F, 0x12, 0x22, 0x38,
      0x35, 0x0C, 0x21, 0x27,
      0x33, 0x36, 0x3B,

      //////////////////////////////////////////////////////////

      WRITE_C8_D8, 0x3A, 0x55, // Color mode, 16-bit

      WRITE_COMMAND_8, 0x11, // SLEEP MODE OUT / BOOSTER ON
      END_WRITE,

      DELAY, 250,
      DELAY, 250,
      DELAY, 250,
      DELAY, 250,
      DELAY, 200,

      BEGIN_WRITE,
      // WRITE_COMMAND_8, 0x13, //
      WRITE_COMMAND_8, 0x29,
      END_WRITE};

  _bus->batchOperation(nt39125_init_operations, sizeof(nt39125_init_operations));

  if (_ips)
  {
    _bus->sendCommand(NT39125_INVON);
  }
}
