#ifndef FS_STUB_H
#define FS_STUB_H

#include "Arduino.h"
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <io.h>

namespace fs
{
    class File
    {
    private:
        FILE *_f;
        bool _isDir;
        char _name[256];
        // Directory traversal
        char _dirPattern[512];
        intptr_t _dirHandle;
        bool _dirOpened;
        struct _finddata_t _findData;

        void _init()
        {
            _f = nullptr;
            _isDir = false;
            _name[0] = '\0';
            _dirPattern[0] = '\0';
            _dirHandle = -1;
            _dirOpened = false;
            memset(&_findData, 0, sizeof(_findData));
        }

        void _extractName(const char *path)
        {
            const char *lastSlash = strrchr(path, '/');
            const char *lastBackslash = strrchr(path, '\\');
            const char *last = (lastSlash > lastBackslash) ? lastSlash : lastBackslash;
            if (last && last[1] != '\0') {
                strncpy(_name, last + 1, sizeof(_name) - 1);
            } else if (path) {
                strncpy(_name, path, sizeof(_name) - 1);
            }
            _name[sizeof(_name) - 1] = '\0';
        }

    public:
        File() { _init(); }

        File(const char *path, const char *mode = "rb")
        {
            _init();
            if (!path || path[0] == '\0') return;

            struct stat st;
            if (stat(path, &st) == 0 && (st.st_mode & _S_IFDIR))
            {
                _isDir = true;
                _extractName(path);
                snprintf(_dirPattern, sizeof(_dirPattern), "%s\\*", path);
                return;
            }

            _f = fopen(path, mode);
            if (_f) _extractName(path);
        }

        File(const File &other)
        {
            _init();
            _f = other._f;
            _isDir = other._isDir;
            strncpy(_name, other._name, sizeof(_name) - 1);
            strncpy(_dirPattern, other._dirPattern, sizeof(_dirPattern) - 1);
            _dirHandle = other._dirHandle;
            _dirOpened = other._dirOpened;
            _findData = other._findData;
            const_cast<File &>(other)._f = nullptr;
            const_cast<File &>(other)._isDir = false;
            const_cast<File &>(other)._dirHandle = -1;
            const_cast<File &>(other)._dirOpened = false;
        }

        File &operator=(const File &other)
        {
            if (this != &other) {
                close();
                _f = other._f;
                _isDir = other._isDir;
                strncpy(_name, other._name, sizeof(_name) - 1);
                strncpy(_dirPattern, other._dirPattern, sizeof(_dirPattern) - 1);
                _dirHandle = other._dirHandle;
                _dirOpened = other._dirOpened;
                _findData = other._findData;
                const_cast<File &>(other)._f = nullptr;
                const_cast<File &>(other)._isDir = false;
                const_cast<File &>(other)._dirHandle = -1;
                const_cast<File &>(other)._dirOpened = false;
            }
            return *this;
        }

        ~File() { close(); }

        operator bool() const { return _f != nullptr || _isDir; }

        size_t read(uint8_t *buf, size_t len)
        {
            if (!_f) return 0;
            return fread(buf, 1, len, _f);
        }

        size_t write(const uint8_t *buf, size_t len)
        {
            if (!_f) return 0;
            return fwrite(buf, 1, len, _f);
        }

        void close()
        {
            if (_f) { fclose(_f); _f = nullptr; }
            if (_dirHandle != -1) { _findclose(_dirHandle); _dirHandle = -1; }
            _isDir = false;
            _dirOpened = false;
        }

        int available()
        {
            if (!_f) return 0;
            long cur = ftell(_f);
            fseek(_f, 0, SEEK_END);
            long end = ftell(_f);
            fseek(_f, cur, SEEK_SET);
            return (int)(end - cur);
        }

        int read()
        {
            if (!_f) return -1;
            return fgetc(_f);
        }

        int peek()
        {
            if (!_f) return -1;
            int c = fgetc(_f);
            if (c != EOF) ungetc(c, _f);
            return c;
        }

        void seek(uint32_t pos)
        {
            if (_f) fseek(_f, (long)pos, SEEK_SET);
        }

        size_t position()
        {
            if (!_f) return 0;
            return (size_t)ftell(_f);
        }

        size_t size()
        {
            if (!_f) return 0;
            long cur = ftell(_f);
            fseek(_f, 0, SEEK_END);
            long sz = ftell(_f);
            fseek(_f, cur, SEEK_SET);
            return (size_t)sz;
        }

        const char *name() { return _name; }

        bool isDirectory() { return _isDir; }

        File openNextFile(const char *mode = NULL)
        {
            File f;
            if (!_isDir) return f;

            if (!_dirOpened)
            {
                _dirHandle = _findfirst(_dirPattern, &_findData);
                if (_dirHandle == -1) return f;
                _dirOpened = true;
            }
            else
            {
                if (_findnext(_dirHandle, &_findData) != 0) return f;
            }

            while (strcmp(_findData.name, ".") == 0 || strcmp(_findData.name, "..") == 0)
            {
                if (_findnext(_dirHandle, &_findData) != 0) return f;
            }

            // Build full path
            char fullPath[512];
            size_t baseLen = strlen(_dirPattern);
            if (baseLen > 1) baseLen--;
            strncpy(fullPath, _dirPattern, baseLen);
            fullPath[baseLen] = '\0';
            strcat(fullPath, _findData.name);

            if (_findData.attrib & _A_SUBDIR)
            {
                f._isDir = true;
                snprintf(f._dirPattern, sizeof(f._dirPattern), "%s\\*", fullPath);
            }
            else
            {
                f._f = fopen(fullPath, mode ? mode : "r");
            }
            strncpy(f._name, _findData.name, sizeof(f._name) - 1);
            f._name[sizeof(f._name) - 1] = '\0';
            return f;
        }

        unsigned long getLastWrite() { return 0; }
    };

    class FS
    {
    public:
        bool begin() { return true; }
        void end() {}
        File open(const char *path, const char *mode) { return File(path, mode); }
        bool exists(const char *path)
        {
            struct stat st;
            return stat(path, &st) == 0;
        }
    };
}

typedef fs::File File;

#endif