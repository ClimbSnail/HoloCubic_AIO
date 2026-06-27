#include "hal_native.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>

#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>

// LVGL 头文件 - 在编译时由 PlatformIO 的 lib 路径提供
#include "lvgl.h"

// 屏幕分辨率（与 ESP32 硬件一致）
#define SDL_HOR_RES 240
#define SDL_VER_RES 240

static SDL_Window   *window   = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture  *texture  = NULL;
lv_color_t   *fb_buf   = NULL;  // 整帧 framebuffer（非 static，供 TFT_eSPI 桩直接写入）

// 键盘动作共享变量
typedef enum {
    HAL_ACTION_NONE = 0,
    HAL_ACTION_RETURN,
    HAL_ACTION_TURN_LEFT,
    HAL_ACTION_TURN_RIGHT,
    HAL_ACTION_UP,
    HAL_ACTION_DOWN,
    HAL_ACTION_SHAKE,
    HAL_ACTION_GO_FORWORD
} hal_action_t;

static volatile hal_action_t g_last_action = HAL_ACTION_NONE;
static CRITICAL_SECTION g_action_lock;

// fb_buf 读写锁：主线程（LVGL flush / TFT 桩）写，SDL 线程（渲染）读
// 非 static，供 TFT_eSPI 桩跨文件访问
CRITICAL_SECTION g_fb_lock;

// SDL 初始化完成标志：主线程等待此标志后才开始 setup()
static volatile bool g_sdl_ready = false;

// LHLXW 子应用状态追踪（供 auto_test 钩子监控）
volatile int  hal_lhlxw_option_num     = -1;  // -1=不在LHLXW, 0-4=当前选项
volatile bool hal_lhlxw_subapp_running = false;

// isCheckAction: 由 HoloCubic_AIO.cpp 定义，为 true 时 LHLXW_process 等 APP
// 会调用 mpu.getAction() 读取键盘动作。在硬件上由 FreeRTOS 定时器置位，
// 在仿真中由 SDL 线程在检测到键盘事件时置位，模拟硬件中断行为。
extern bool isCheckAction;

// ========================
// SDL 线程：窗口创建 + 事件处理 + 渲染 + auto_test 推进
// 对应真实硬件中的 SPI 总线 + GPIO 中断
// 所有 SDL 操作集中在此线程，满足 SDL 线程安全要求
// ========================
void hal_native_sdl_thread(void)
{
    hal_native_init();

    printf("[HAL] SDL2 initialized: %dx%d window\n", SDL_HOR_RES, SDL_VER_RES);
    printf("[HAL] Keyboard: LEFT/RIGHT=switch, UP/DOWN=vertical, SPACE=enter, BACKSPACE=back, ENTER=shake\n");
    fflush(stdout);

    g_sdl_ready = true;

    while (1) {
        // SDL 事件处理（键盘 → g_last_action，模拟 GPIO 中断）
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            }
            if (event.type == SDL_KEYDOWN) {
                EnterCriticalSection(&g_action_lock);
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:     g_last_action = HAL_ACTION_TURN_LEFT;  break;
                    case SDLK_RIGHT:    g_last_action = HAL_ACTION_TURN_RIGHT; break;
                    case SDLK_UP:       g_last_action = HAL_ACTION_UP;         break;
                    case SDLK_DOWN:     g_last_action = HAL_ACTION_DOWN;       break;
                    case SDLK_SPACE:    g_last_action = HAL_ACTION_GO_FORWORD; break;
                    case SDLK_RETURN:   g_last_action = HAL_ACTION_SHAKE;      break;
                    case SDLK_BACKSPACE: g_last_action = HAL_ACTION_RETURN;    break;
                    default: break;
                }
                LeaveCriticalSection(&g_action_lock);
                isCheckAction = true;
            }
        }

        // auto_test 钩子：在 SDL 线程中驱动，不依赖主线程（FreeRTOS 调度器）
        hal_native_auto_test_hook();

        // 渲染 fb_buf 到 SDL 窗口（模拟 SPI 总线刷新屏幕）
        EnterCriticalSection(&g_fb_lock);
        SDL_UpdateTexture(texture, NULL, fb_buf, SDL_HOR_RES * sizeof(lv_color_t));
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        LeaveCriticalSection(&g_fb_lock);

        SDL_Delay(5);
    }
}

void hal_native_init(void)
{
    SDL_Init(SDL_INIT_VIDEO);

    InitializeCriticalSection(&g_action_lock);
    InitializeCriticalSection(&g_fb_lock);

    window = SDL_CreateWindow("HoloCubic_AIO Simulator",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              SDL_HOR_RES * 2, SDL_VER_RES * 2,
                              SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_RGB565,
                                SDL_TEXTUREACCESS_STREAMING,
                                SDL_HOR_RES, SDL_VER_RES);

    fb_buf = (lv_color_t *)malloc(SDL_HOR_RES * SDL_VER_RES * sizeof(lv_color_t));
    memset(fb_buf, 0, SDL_HOR_RES * SDL_VER_RES * sizeof(lv_color_t));
}

bool hal_native_is_ready(void)
{
    return g_sdl_ready;
}

bool hal_native_loop(void)
{
    // 主线程不再处理 SDL，仅做最小 yield
    // SDL 渲染和事件处理已移至 hal_native_sdl_thread()
    SDL_Delay(1);
    return true;
}

void hal_native_cleanup(void)
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    free(fb_buf);
    DeleteCriticalSection(&g_fb_lock);
    DeleteCriticalSection(&g_action_lock);
    SDL_Quit();
}

void hal_display_flush(int x1, int y1, int x2, int y2, const void *color_p)
{
    uint32_t w = (x2 - x1 + 1);
    uint32_t h = (y2 - y1 + 1);
    const lv_color_t *src = (const lv_color_t *)color_p;

    EnterCriticalSection(&g_fb_lock);
    for (uint32_t y = 0; y < h; y++) {
        uint32_t dst_offset = (y1 + y) * SDL_HOR_RES + x1;
        uint32_t src_offset = y * w;
        memcpy(&fb_buf[dst_offset], &src[src_offset], w * sizeof(lv_color_t));
    }
    LeaveCriticalSection(&g_fb_lock);
}

void hal_lvgl_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    EnterCriticalSection(&g_fb_lock);
    for (uint32_t y = 0; y < h; y++) {
        uint32_t dst_offset = (area->y1 + y) * SDL_HOR_RES + area->x1;
        uint32_t src_offset = y * w;
        memcpy(&fb_buf[dst_offset], &color_p[src_offset], w * sizeof(lv_color_t));
    }
    LeaveCriticalSection(&g_fb_lock);

    lv_disp_flush_ready(disp);
}

// 供 hal_imu.c 调用的动作获取函数
int hal_native_get_key_action(void)
{
    EnterCriticalSection(&g_action_lock);
    hal_action_t action = (hal_action_t)g_last_action;
    g_last_action = HAL_ACTION_NONE;
    LeaveCriticalSection(&g_action_lock);
    return (int)action;
}

// 自动测试：向模拟器注入动作
void hal_native_inject_action(int action)
{
    EnterCriticalSection(&g_action_lock);
    g_last_action = (hal_action_t)action;
    LeaveCriticalSection(&g_action_lock);
}

// 自动测试钩子（弱函数，默认空实现）
// 由 auto_test.cpp 覆盖以实现自动测试
__attribute__((weak)) void hal_native_auto_test_hook(void)
{
}