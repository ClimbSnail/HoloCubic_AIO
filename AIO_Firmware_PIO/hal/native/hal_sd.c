#include "hal_native.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <direct.h>

#define SDCARD_DIR "./sdcard"

static bool sd_initialized = false;

bool hal_sd_init(void)
{
    if (sd_initialized) {
        return true;
    }

    // 创建本地 sdcard 目录（如果不存在）
    struct stat st = {0};
    if (stat(SDCARD_DIR, &st) == -1) {
        _mkdir(SDCARD_DIR);
    }

    sd_initialized = true;
    printf("[SD] Simulated SD card initialized at '%s'\n", SDCARD_DIR);
    return true;
}

bool hal_sd_read_file(const char *path, uint8_t *buf, size_t *len)
{
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", SDCARD_DIR, path);

    FILE *fp = fopen(full_path, "rb");
    if (!fp) {
        printf("[SD] Read failed: %s\n", full_path);
        return false;
    }

    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (*len < file_size) {
        file_size = *len;
    }

    size_t read_size = fread(buf, 1, file_size, fp);
    fclose(fp);

    *len = read_size;
    return true;
}

bool hal_sd_write_file(const char *path, const uint8_t *buf, size_t len)
{
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", SDCARD_DIR, path);

    FILE *fp = fopen(full_path, "wb");
    if (!fp) {
        printf("[SD] Write failed: %s\n", full_path);
        return false;
    }

    size_t written = fwrite(buf, 1, len, fp);
    fclose(fp);

    return (written == len);
}

bool hal_sd_exists(const char *path)
{
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", SDCARD_DIR, path);

    struct stat st;
    return (stat(full_path, &st) == 0);
}