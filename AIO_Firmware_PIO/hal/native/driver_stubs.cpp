#include "common.h"
#include "hal_native.h"
#include "sim_data_path.h"
#include <cstdio>
#include <cstring>
#include <io.h>
#include "SPIFFS.h"

// ==================== Display ====================
void Display::init(uint8_t rotation, uint8_t backLight)
{
    (void)rotation;
    (void)backLight;

    printf("[DISP] lv_init()...\n");
    lv_init();

    static lv_color_t buf[SCREEN_HOR_RES * 24];
    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, SCREEN_HOR_RES * 24);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_HOR_RES;
    disp_drv.ver_res = SCREEN_VER_RES;
    disp_drv.flush_cb = hal_lvgl_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    printf("[DISP] LVGL initialized, driver registered (240x240)\n");
}

void Display::routine()
{
    static unsigned long routine_count = 0;
    routine_count++;
    if (routine_count % 1000 == 0) {
        printf("[DISP] routine() frame=%lu\n", routine_count);
    }
    AIO_LVGL_OPERATE_LOCK(lv_timer_handler());
}

void Display::setBackLight(float val)
{
    (void)val;
}

// ==================== Pixel ====================
void Pixel::init()
{
}

Pixel& Pixel::setRGB(int r, int g, int b)
{
    (void)r;
    (void)g;
    (void)b;
    return *this;
}

Pixel& Pixel::setHVS(uint8_t ih, uint8_t is, uint8_t iv)
{
    (void)ih;
    (void)is;
    (void)iv;
    return *this;
}

Pixel& Pixel::fill_rainbow(int min_r, int max_r, int min_g, int max_g, int min_b, int max_b)
{
    (void)min_r;
    (void)max_r;
    (void)min_g;
    (void)max_g;
    (void)min_b;
    (void)max_b;
    return *this;
}

Pixel& Pixel::setBrightness(float duty)
{
    (void)duty;
    return *this;
}

// ==================== Ambient ====================
void Ambient::init(int mode)
{
    (void)mode;
}

// ==================== SdCard ====================

static char g_sdBasePath[512] = "";

static void computeSdBasePath()
{
    static bool computed = false;
    if (computed) return;
    computed = true;
    snprintf(g_sdBasePath, sizeof(g_sdBasePath), "%s", sim_data_get_sd_path());
}

static void toNativePath(const char *virtPath, char *nativePath, size_t size)
{
    computeSdBasePath();
    if (virtPath[0] == 'S' && virtPath[1] == ':')
        virtPath += 2;
    if (virtPath[0] == '/' || virtPath[0] == '\\')
        virtPath += 1;
    snprintf(nativePath, size, "%s\\%s", g_sdBasePath, virtPath);
    for (char *p = nativePath; *p; ++p)
        if (*p == '/') *p = '\\';
}

void SdCard::init()
{
}

void SdCard::listDir(const char *dirname, uint8_t levels)
{
    (void)dirname;
    (void)levels;
}

File_Info *SdCard::listDir(const char *dirname)
{
    char nativePath[512];
    toNativePath(dirname, nativePath, sizeof(nativePath));

    printf("[SDCARD] listDir(%s) -> %s\n", dirname, nativePath);

    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%s\\*", nativePath);

    struct _finddata_t fdata;
    intptr_t handle = _findfirst(pattern, &fdata);
    if (handle == -1)
    {
        printf("[SDCARD] _findfirst failed for %s\n", pattern);
        return NULL;
    }

    int dir_len = strlen(dirname) + 1;

    File_Info *head_file = (File_Info *)malloc(sizeof(File_Info));
    head_file->file_type = FILE_TYPE_FOLDER;
    head_file->file_name = (char *)malloc(dir_len);
    strncpy(head_file->file_name, dirname, dir_len - 1);
    head_file->file_name[dir_len - 1] = 0;
    head_file->front_node = NULL;
    head_file->next_node = NULL;

    File_Info *file_node = head_file;
    int file_count = 0;

    do
    {
        if (strcmp(fdata.name, ".") == 0 || strcmp(fdata.name, "..") == 0)
            continue;

        int filename_len = strlen(fdata.name);
        if (filename_len > 128 - 10)
            continue;

        file_node->next_node = (File_Info *)malloc(sizeof(File_Info));
        file_node->next_node->front_node = file_node;
        file_node = file_node->next_node;

        file_node->file_name = (char *)malloc(filename_len + 1);
        strncpy(file_node->file_name, fdata.name, filename_len);
        file_node->file_name[filename_len] = 0;
        file_node->next_node = NULL;

        if (fdata.attrib & _A_SUBDIR)
        {
            file_node->file_type = FILE_TYPE_FOLDER;
        }
        else
        {
            file_node->file_type = FILE_TYPE_FILE;
        }
        file_count++;

    } while (_findnext(handle, &fdata) == 0);

    _findclose(handle);

    if (NULL != head_file->next_node)
    {
        file_node->next_node = head_file->next_node;
        head_file->next_node->front_node = file_node;
    }

    printf("[SDCARD] listDir found %d files\n", file_count);
    return head_file;
}

boolean SdCard::deleteFile(const String &path)
{
    char nativePath[512];
    toNativePath(path.c_str(), nativePath, sizeof(nativePath));
    printf("[SDCARD] deleteFile(%s)\n", nativePath);
    return remove(nativePath) == 0;
}

File SdCard::open(const String &path, const char *mode)
{
    return SD.open(path, mode);
}

// ==================== Stub for LHLXW emoji (excluded) ====================
void emoji_process(lv_obj_t *ym)
{
    (void)ym;
}

// ==================== IMU ====================
IMU::IMU()
{
    action_info.active = ACTIVE_TYPE::UNKNOWN;
    action_info.isValid = 0;
    action_info.long_time = false;
    for (int i = 0; i < ACTION_HISTORY_BUF_LEN; i++) {
        act_info_history[i] = ACTIVE_TYPE::UNKNOWN;
    }
    act_info_history_ind = ACTION_HISTORY_BUF_LEN - 1;
    order = 0;
}

void IMU::init(uint8_t order, uint8_t auto_calibration, SysMpuConfig *mpu_cfg)
{
    (void)order;
    (void)auto_calibration;
    (void)mpu_cfg;
}

void IMU::setOrder(uint8_t order)
{
    (void)order;
}

bool IMU::Encoder_GetIsPush(void)
{
    return false;
}

ImuAction* IMU::getAction(void)
{
    int key_action = hal_imu_get_action();
    // 先写 active 再写 isValid，防止主循环读到中间状态
    switch (key_action) {
        case 1:  action_info.active = RETURN;      break;
        case 2:  action_info.active = TURN_LEFT;   break;
        case 3:  action_info.active = TURN_RIGHT;  break;
        case 4:  action_info.active = UP;          break;
        case 5:  action_info.active = DOWN;        break;
        case 6:  action_info.active = SHAKE;       break;
        case 7:  action_info.active = GO_FORWORD;  break;
        default: action_info.active = UNKNOWN;     break;
    }
    action_info.isValid = (key_action != 0);
    return &action_info;
}

void IMU::getVirtureMotion6(ImuAction *action_info)
{
    (void)action_info;
}

// ==================== FlashFS ====================
FlashFS::FlashFS()
{
}

FlashFS::~FlashFS()
{
}

uint16_t FlashFS::readFile(const char *path, uint8_t *info)
{
    if (!info) return 0;
    File file = SPIFFS.open(path);
    if (!file || file.isDirectory())
    {
        Serial.printf("[FlashFS] readFile(%s) failed\n", path);
        return 0;
    }
    uint16_t ret = 0;
    while (file.available())
    {
        ret += file.read(info + ret, 15);
    }
    file.close();
    Serial.printf("[FlashFS] readFile(%s) OK, %d bytes\n", path, ret);
    return ret;
}

void FlashFS::writeFile(const char *path, const char *message)
{
    File file = SPIFFS.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.printf("[FlashFS] writeFile(%s) failed\n", path);
        return;
    }
    file.write((const uint8_t *)message, strlen(message));
    file.close();
}

void FlashFS::appendFile(const char *path, const char *message)
{
    File file = SPIFFS.open(path, FILE_APPEND);
    if (!file)
    {
        Serial.printf("[FlashFS] appendFile(%s) failed\n", path);
        return;
    }
    file.write((const uint8_t *)message, strlen(message));
    file.close();
}

void FlashFS::renameFile(const char *src, const char *dst)
{
    char srcBuf[512], dstBuf[512];
    snprintf(srcBuf, sizeof(srcBuf), "%s%s", sim_data_get_spiffs_path(), src);
    snprintf(dstBuf, sizeof(dstBuf), "%s%s", sim_data_get_spiffs_path(), dst);
    ::rename(srcBuf, dstBuf);
}

void FlashFS::deleteFile(const char *path)
{
    SPIFFS.remove(path);
}

// ==================== Network ====================
Network::Network()
{
}

void Network::search_wifi(void)
{
}

boolean Network::start_conn_wifi(const char *ssid, const char *password)
{
    (void)ssid;
    (void)password;
    return true;
}

boolean Network::end_conn_wifi(void)
{
    return CONN_SUCC;
}

boolean Network::close_wifi(void)
{
    return true;
}

boolean Network::open_ap(const char *ap_ssid, const char *ap_password)
{
    (void)ap_ssid;
    (void)ap_password;
    return true;
}

// ==================== 全局函数 ====================
bool set_rgb_and_run(RgbParam *rgb_setting, LED_RUN_MODE mode)
{
    (void)rgb_setting;
    (void)mode;
    return true;
}

void rgb_stop(void)
{
}

bool analyseParam(char *info, int argc, char **argv)
{
    int cnt; // 记录解析到第几个参数
    for (cnt = 0; cnt < argc; ++cnt)
    {
        argv[cnt] = info;
        while (*info != '\n' && *info != '\0')
        {
            ++info;
        }
        // 兼容 Windows 文本模式产生的 \r\n 换行符
        if (info > argv[cnt] && *(info - 1) == '\r')
        {
            *(info - 1) = 0;
        }
        *info = 0;
        ++info;
    }
    return true;
}

// ==================== 全局变量 ====================
const char *AP_SSID = "HoloCubic_AIO";

const char *active_type_info[] = {
    "TURN_RIGHT",
    "RETURN",
    "TURN_LEFT",
    "UP",
    "DOWN",
    "GO_FORWORD",
    "SHAKE",
    "UNKNOWN"
};

// ==================== FreeRTOS 回调 ====================
extern "C" void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                              StackType_t **ppxIdleTaskStackBuffer,
                                              configSTACK_DEPTH_TYPE *puxIdleTaskStackSize)
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

extern "C" void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                               StackType_t **ppxTimerTaskStackBuffer,
                                               configSTACK_DEPTH_TYPE *pulTimerTaskStackSize)
{
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

// ==================== TJpg_Decoder 桩 ====================
// TJpgDec is defined in lib/TJpg_Decoder (now included via platformio.ini)

// ==================== esp_system 桩 ====================
#include "esp_system.h"
#include <cstdlib>

extern "C" void esp_restart(void)
{
    exit(0);
}

extern "C" uint32_t esp_random(void)
{
    return (uint32_t)rand();
}

extern "C" uint32_t esp_get_free_heap_size(void)
{
    return 64 * 1024 * 1024;
}

extern "C" uint32_t esp_get_minimum_free_heap_size(void)
{
    return 32 * 1024 * 1024;
}

// ==================== heap_caps 桩 ====================
extern "C" void *heap_caps_malloc(size_t size, uint32_t caps)
{
    (void)caps;
    return malloc(size);
}

// ==================== SD 卡桩 ====================
#include "SD.h"

void release_file_info(File_Info *info)
{
    if (NULL == info)
        return;

    File_Info *node = info->next_node;
    while (node != NULL && node != info->next_node)
    {
        File_Info *next = node->next_node;
        free(node->file_name);
        free(node);
        node = next;
    }
    free(info->file_name);
    free(info);
}