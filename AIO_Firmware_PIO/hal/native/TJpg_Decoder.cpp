#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "TJpg_Decoder.h"
#include "SD.h"

TJpg_Decoder TJpgDec;

// 解码完成标志（供自动测试框架钩子读取）
volatile int g_jpg_decode_done = 0;

extern "C" int hal_native_is_jpg_decode_done(void)
{
    int result = g_jpg_decode_done;
    g_jpg_decode_done = 0;
    return result;
}

TJpg_Decoder::TJpg_Decoder() {}
TJpg_Decoder::~TJpg_Decoder() {}

void TJpg_Decoder::setJpgScale(uint8_t scale)
{
    jpgScale = scale;
}

void TJpg_Decoder::setCallback(SketchCallback sketchCallback)
{
    tft_output = sketchCallback;
}

void TJpg_Decoder::setSwapBytes(bool swap)
{
    _swap = swap;
}

static inline uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

JRESULT TJpg_Decoder::drawSdJpg(int32_t x, int32_t y, File inFile)
{
    if (!inFile) {
        Serial.println(F("[TJPG] drawSdJpg(File): invalid file"));
        return JDR_INP;
    }

    size_t fileSize = inFile.size();
    if (fileSize == 0) {
        Serial.println(F("[TJPG] drawSdJpg(File): empty file"));
        inFile.close();
        return JDR_INP;
    }

    uint8_t *fileData = new uint8_t[fileSize];
    if (!fileData) {
        Serial.println(F("[TJPG] drawSdJpg(File): alloc failed"));
        inFile.close();
        return JDR_INP;
    }

    size_t bytesRead = inFile.read(fileData, fileSize);
    inFile.close();

    if (bytesRead != fileSize) {
        Serial.printf("[TJPG] drawSdJpg(File): read %zu / %zu bytes\n", bytesRead, fileSize);
        delete[] fileData;
        return JDR_INP;
    }

    int width, height, channels;
    unsigned char *img = stbi_load_from_memory(fileData, (int)fileSize, &width, &height, &channels, 3);
    delete[] fileData;

    if (!img) {
        Serial.printf("[TJPG] stbi_load_from_memory failed: %s\n", stbi_failure_reason());
        return JDR_INP;
    }

    Serial.printf("[TJPG] decoded %dx%d channels=%d\n", width, height, channels);

    if (tft_output) {
        uint16_t *rgb565 = new uint16_t[width * height];
        for (int i = 0; i < width * height; i++) {
            rgb565[i] = rgb888_to_rgb565(img[i * 3], img[i * 3 + 1], img[i * 3 + 2]);
        }
        tft_output(x, y, width, height, rgb565);
        delete[] rgb565;
    }

    stbi_image_free(img);
    g_jpg_decode_done = 1;
    return JDR_OK;
}

JRESULT TJpg_Decoder::drawSdJpg(int32_t x, int32_t y, const char *pFilename)
{
    if (!SD.exists(pFilename)) {
        Serial.printf("[TJPG] Jpeg file not found: %s\n", pFilename);
        return JDR_INP;
    }
    return drawSdJpg(x, y, SD.open(pFilename, FILE_READ));
}

JRESULT TJpg_Decoder::drawSdJpg(int32_t x, int32_t y, const String& pFilename)
{
    return drawSdJpg(x, y, pFilename.c_str());
}

JRESULT TJpg_Decoder::drawJpg(int32_t x, int32_t y, const uint8_t array[], uint32_t array_size)
{
    if (!array || array_size == 0) {
        Serial.println(F("[TJPG] drawJpg(array): invalid input"));
        return JDR_INP;
    }

    int width, height, channels;
    unsigned char *img = stbi_load_from_memory(array, (int)array_size, &width, &height, &channels, 3);

    if (!img) {
        Serial.printf("[TJPG] drawJpg(array): stbi_load_from_memory failed: %s\n", stbi_failure_reason());
        return JDR_INP;
    }

    if (tft_output) {
        uint16_t *rgb565 = new uint16_t[width * height];
        for (int i = 0; i < width * height; i++) {
            rgb565[i] = rgb888_to_rgb565(img[i * 3], img[i * 3 + 1], img[i * 3 + 2]);
        }
        tft_output(x, y, width, height, rgb565);
        delete[] rgb565;
    }

    stbi_image_free(img);
    g_jpg_decode_done = 1;
    return JDR_OK;
}