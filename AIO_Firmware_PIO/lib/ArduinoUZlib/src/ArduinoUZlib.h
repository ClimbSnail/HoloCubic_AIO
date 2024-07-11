
#ifndef _ARDUINO_UZLIB_
#define _ARDUINO_UZLIB_
#include <Arduino.h>
#include "uzlib.h"

class ArduinoUZlib {
public:
    ArduinoUZlib();
    static int32_t decompress(uint8_t *in_buf, uint32_t in_size, uint8_t *&dest, uint32_t &out_size);
};

#endif