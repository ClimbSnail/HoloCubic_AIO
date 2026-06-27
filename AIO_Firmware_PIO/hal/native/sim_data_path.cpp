#include "sim_data_path.h"
#include <cstdio>
#include <cstring>

extern "C" {
    __declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void* hModule, char* lpFilename, unsigned long nSize);
}

static char g_sdPath[512] = "";
static char g_spiffsPath[512] = "";

static void computeBasePath(void)
{
    static bool computed = false;
    if (computed) return;
    computed = true;

    char exePath[512];
    GetModuleFileNameA(NULL, exePath, sizeof(exePath));

    char *lastSlash = strrchr(exePath, '\\');
    if (lastSlash) *lastSlash = '\0';
    lastSlash = strrchr(exePath, '\\');
    if (lastSlash) *lastSlash = '\0';
    lastSlash = strrchr(exePath, '\\');
    if (lastSlash) *lastSlash = '\0';
    lastSlash = strrchr(exePath, '\\');
    if (lastSlash) *lastSlash = '\0';
    lastSlash = strrchr(exePath, '\\');
    if (lastSlash) *lastSlash = '\0';

    snprintf(g_sdPath, sizeof(g_sdPath), "%s\\sim_data\\sd", exePath);
    snprintf(g_spiffsPath, sizeof(g_spiffsPath), "%s\\sim_data\\spiffs", exePath);
}

const char* sim_data_get_sd_path(void)
{
    computeBasePath();
    return g_sdPath;
}

const char* sim_data_get_spiffs_path(void)
{
    computeBasePath();
    return g_spiffsPath;
}