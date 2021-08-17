Arduino TJpg_Decoder library
===========

This Arduino library supports the rendering of Jpeg files stored both on SD card and in arrays within program memory (FLASH) onto a TFT display. In addition images stored in the SPIFFS Flash filing system or "PROGMEM" arrays can be used with the ESP8266 and ESP32 processors.

The library works on the Arduino Due, ESP32 and ESP8266 (e.g. NodeMCU 1.0).

Jpeg files in the "Progressive" format (where image data is compressed in multiple passes with progressively higher detail) are not supported since this would require much more memory, or too many Inverse Discrete Cosine Transform's for typical embedded systems.

When storing the jpeg in a memory array bear in mind the Arduino has a maximum 32767 byte limit for the maximum size of an array (32 KBytes minus 1 byte).

The decompression of Jpeg images needs more RAM than an UNO provides, thus this library is targetted at processors with more RAM. The library has been tested with ESP8266/ESP32 based boards.

On a Mega the number of images stored in FLASH must be limited because it they are large enough to push the executable code start over the 64K 16 bit address limit then the Mega will fail to boot even though the sketch compiles and uploads correctly. This is a limitation imposed by the Arduino environment not this library!  The Arduino Mega is not recommended as it does not reliably decode some jpeg images possibly due to a shortage of RAM.  The Due will work fine with much bigger image sets in FLASH.

This library uses the TJpgDec decompressor engine detailed here:
http://elm-chan.org/fsw/tjpgd/00index.html
TJpgDec is a generic JPEG image decompressor module that highly optimized for small embedded systems.