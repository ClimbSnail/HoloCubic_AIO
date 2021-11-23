/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 */
#include "Arduino_DataBus.h"

Arduino_DataBus::Arduino_DataBus() {}

void Arduino_DataBus::writeC8D8(uint8_t c, uint8_t d)
{
  writeCommand(c);
  write(d);
}

void Arduino_DataBus::writeC8D16(uint8_t c, uint16_t d)
{
  writeCommand(c);
  write16(d);
}

void Arduino_DataBus::writeC8D16D16(uint8_t c, uint16_t d1, uint16_t d2)
{
  writeCommand(c);
  write16(d1);
  write16(d2);
}

void Arduino_DataBus::sendCommand(uint8_t c)
{
  beginWrite();
  writeCommand(c);
  endWrite();
}

void Arduino_DataBus::sendCommand16(uint16_t c)
{
  beginWrite();
  writeCommand16(c);
  endWrite();
}

void Arduino_DataBus::sendData(uint8_t d)
{
  beginWrite();
  write(d);
  endWrite();
}

void Arduino_DataBus::sendData16(uint16_t d)
{
  beginWrite();
  write16(d);
  endWrite();
}

void Arduino_DataBus::batchOperation(uint8_t batch[], size_t len)
{
  for (size_t i = 0; i < len; ++i)
  {
    uint8_t l = 0;
    switch (batch[i])
    {
    case BEGIN_WRITE:
      beginWrite();
      break;
    case WRITE_C8_D16:
      l++;
    case WRITE_C8_D8:
      l++;
    case WRITE_COMMAND_8:
      writeCommand(batch[++i]);
      break;
    case WRITE_C16_D16:
      l = 2;
    case WRITE_COMMAND_16:
      _data16.msb = batch[++i];
      _data16.lsb = batch[++i];
      writeCommand16(_data16.value);
      break;
    case WRITE_DATA_8:
      l = 1;
      break;
    case WRITE_DATA_16:
      l = 2;
      break;
    case WRITE_BYTES:
      l = batch[++i];
      break;
    case END_WRITE:
      endWrite();
      break;
    case DELAY:
      delay(batch[++i]);
      break;
    default:
      printf("Unknown operation id at %d: %d", i, batch[i]);
      break;
    }
    while (l--)
    {
      write(batch[++i]);
    }
  }
}

#if !defined(LITTLE_FOOT_PRINT)
void Arduino_DataBus::writeIndexedPixels(uint8_t *data, uint16_t *idx, uint32_t len)
{
  while (len--)
  {
    write16(idx[*(data++)]);
  }
}

void Arduino_DataBus::writeIndexedPixelsDouble(uint8_t *data, uint16_t *idx, uint32_t len)
{
  uint8_t *d = data;
  while (len--)
  {
    _data16.value = idx[*(d++)];
    write(_data16.msb);
    write(_data16.lsb);
    write(_data16.msb);
    write(_data16.lsb);
  }
}
#endif // !defined(LITTLE_FOOT_PRINT)
