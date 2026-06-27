#ifndef SD_STUB_H
#define SD_STUB_H

#include "FS.h"

class SDClass
{
public:
    bool begin(uint8_t cs = 0, uint32_t freq = 0) { (void)cs; (void)freq; return true; }
    File open(const char *path, const char *mode = "r");
    File open(const String &path, const char *mode = "r") { return open(path.c_str(), mode); }
    bool exists(const char *path);
    bool exists(const String &path) { return exists(path.c_str()); }
};

extern SDClass SD;

#ifdef __cplusplus
extern "C" {
#endif
const char* sd_get_base_path(void);
#ifdef __cplusplus
}
#endif

#endif /* SD_STUB_H */