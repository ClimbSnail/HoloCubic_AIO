#include "SD.h"
#include "sim_data_path.h"
#include <cstdio>
#include <cstring>
#include <direct.h>
#include <sys/stat.h>

SDClass SD;

static char g_sdBasePath[512] = "";

static void ensureSdBasePath()
{
    if (g_sdBasePath[0] != '\0') return;
    snprintf(g_sdBasePath, sizeof(g_sdBasePath), "%s", sim_data_get_sd_path());
    _mkdir(g_sdBasePath);
}

const char* sd_get_base_path(void)
{
    ensureSdBasePath();
    return g_sdBasePath;
}

static void toNativePath(const char *virtPath, char *nativePath, size_t size)
{
    ensureSdBasePath();
    if (virtPath[0] == '/') virtPath++;
    snprintf(nativePath, size, "%s\\%s", g_sdBasePath, virtPath);
    for (char *p = nativePath; *p; ++p)
        if (*p == '/') *p = '\\';
}

bool SDClass::exists(const char *path)
{
    char nativePath[512];
    toNativePath(path, nativePath, sizeof(nativePath));
    printf("[SD] exists(%s) -> %s\n", path, nativePath);
    struct stat st;
    int result = stat(nativePath, &st);
    printf("[SD] exists result=%d\n", result);
    return result == 0;
}

File SDClass::open(const char *path, const char *mode)
{
    char nativePath[512];
    toNativePath(path, nativePath, sizeof(nativePath));
    printf("[SD] open(%s, %s) -> %s\n", path, mode, nativePath);
    File f(nativePath, mode);
    printf("[SD] open result=%d\n", (bool)f);
    return f;
}