#ifndef TJpg_Decoder_NATIVE_H
#define TJpg_Decoder_NATIVE_H

#include "Arduino.h"
#include "SD.h"

typedef enum {
    JDR_OK = 0,
    JDR_INP = 2,
} JRESULT;

typedef bool (*SketchCallback)(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *data);

class TJpg_Decoder {
public:
    TJpg_Decoder();
    ~TJpg_Decoder();

    void setJpgScale(uint8_t scale);
    void setCallback(SketchCallback sketchCallback);

    JRESULT drawSdJpg(int32_t x, int32_t y, const char *pFilename);
    JRESULT drawSdJpg(int32_t x, int32_t y, const String& pFilename);
    JRESULT drawSdJpg(int32_t x, int32_t y, File inFile);

    JRESULT drawJpg(int32_t x, int32_t y, const uint8_t array[], uint32_t array_size);

    void setSwapBytes(bool swap);

private:
    SketchCallback tft_output = nullptr;
    uint8_t jpgScale = 0;
    bool _swap = false;
};

extern TJpg_Decoder TJpgDec;

#endif