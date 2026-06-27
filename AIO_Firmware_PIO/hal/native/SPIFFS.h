#ifndef SPIFFS_STUB_H
#define SPIFFS_STUB_H

#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <direct.h>
#include "FS.h"

#undef FILE_WRITE
#define FILE_WRITE "wb"
#undef FILE_READ
#define FILE_READ  "rb"
#undef FILE_APPEND
#define FILE_APPEND "ab"

class SPIFFSClass
{
private:
    static const char *_nativePath(const char *spiffsPath)
    {
        static char buf[512];
        if (!spiffsPath || spiffsPath[0] == '\0') {
            snprintf(buf, sizeof(buf), "%s", _basePath);
        } else {
            snprintf(buf, sizeof(buf), "%s%s", _basePath, spiffsPath);
        }
        return buf;
    }

public:
    static const char *_basePath;
    bool begin(bool formatOnFail = false)
    {
        (void)formatOnFail;
        extern void computeBasePath();
        computeBasePath();
        _mkdir(_basePath);
        return true;
    }

    void end() {}

    bool format() { return true; }

    File open(const char *path)
    {
        return File(_nativePath(path), "rb");
    }

    File open(const char *path, const char *mode)
    {
        return File(_nativePath(path), mode);
    }

    bool exists(const char *path)
    {
        struct stat st;
        return stat(_nativePath(path), &st) == 0;
    }

    bool remove(const char *path)
    {
        return ::remove(_nativePath(path)) == 0;
    }

    bool mkdir(const char *path)
    {
        return _mkdir(_nativePath(path)) == 0;
    }

    bool rmdir(const char *path)
    {
        return _rmdir(_nativePath(path)) == 0;
    }
};

extern SPIFFSClass SPIFFS;

#endif