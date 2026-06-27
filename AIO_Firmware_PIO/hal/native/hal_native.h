#ifndef HAL_NATIVE_H
#define HAL_NATIVE_H

#include <stdint.h>
#include <stdbool.h>

// 阻止 Windows SDK 引入与项目 stub 冲突的类型定义
// WIN32_LEAN_AND_MEAN 排除 rpcndr.h（定义了 boolean）和 dlgs.h（定义了 CRGB）等
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "lvgl.h"

#ifdef __cplusplus
#include "MPU6050.h"
extern "C" {
#endif

// 初始化所有 HAL 模拟层（SDL 窗口、输入设备等）
// 注意：由 SDL 线程调用，不阻塞
void hal_native_init(void);

// SDL 线程入口：窗口创建 + 事件处理 + 渲染 + auto_test 推进
// 对应真实硬件中的 SPI 总线 + GPIO 中断，在独立线程中运行
// 主线程调用此函数后会阻塞在 while(1) 循环中
void hal_native_sdl_thread(void);

// 检查 SDL 是否初始化完成（主线程轮询此标志，就绪后才开始 setup()）
bool hal_native_is_ready(void);

// 主循环：处理 SDL 事件 + LVGL 定时器（单次迭代，返回 false 表示退出）
// 当前为 no-op，SDL 渲染和事件处理已移至 hal_native_sdl_thread()
bool hal_native_loop(void);

// 清理 SDL 资源
void hal_native_cleanup(void);

// 显示 flush 回调：将 LVGL 帧buffer渲染到 SDL2 窗口
void hal_display_flush(int x1, int y1, int x2, int y2, const void *color_p);

// LVGL 显示刷新回调（符合 LVGL v8 标准签名）
void hal_lvgl_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);

// IMU 模拟：获取键盘生成的模拟动作数据
// 返回: 0=无动作, 1=返回, 2=左翻, 3=右翻, 4=上, 5=下, 6=摇晃, 7=前倾/进入
int hal_imu_get_action(void);

// 获取 IMU 原始数据（加速度 + 陀螺仪）
void hal_imu_get_motion(int16_t *ax, int16_t *ay, int16_t *az,
                        int16_t *gx, int16_t *gy, int16_t *gz);

// 环境光传感器：返回模拟光照值（lux）
unsigned int hal_ambient_get_lux(void);

// SD 卡模拟初始化
bool hal_sd_init(void);

// SD 卡文件操作：读取文件
bool hal_sd_read_file(const char *path, uint8_t *buf, size_t *len);

// SD 卡文件操作：写入文件
bool hal_sd_write_file(const char *path, const uint8_t *buf, size_t len);

// SD 卡文件操作：检查文件是否存在
bool hal_sd_exists(const char *path);

// RGB LED 模拟
void hal_rgb_set_color(uint8_t r, uint8_t g, uint8_t b);

// 自动测试：向模拟器注入动作（替代键盘输入）
void hal_native_inject_action(int action);

// 自动测试钩子（弱函数，由 auto_test.cpp 覆盖）
// 在 hal_native_loop() 中每帧调用，可在此注入测试动作
void hal_native_auto_test_hook(void);

// ============================
// Framebuffer 共享变量（供 TFT_eSPI 桩直接写入，模拟 SPI 总线）
// fb_buf: 240x240 RGB565 整帧缓冲，由 hal_display.c 分配
// g_fb_lock: 读写锁，TFT 桩写入和 SDL 渲染时需持有
// ============================
#ifdef __cplusplus
extern "C" {
#endif
extern lv_color_t   *fb_buf;
extern CRITICAL_SECTION g_fb_lock;
#ifdef __cplusplus
}
#endif

// 检查 JPEG 解码是否完成（由 TJpg_Decoder 设置，自动测试框架读取）
// 返回 1 表示自上次查询以来有新的解码完成，读取后自动清零
int hal_native_is_jpg_decode_done(void);

// 通知 auto_test 框架：服务器路由已注册完毕
// 由 server.cpp 在 start_web_config() 之后调用
void hal_native_server_routes_ready(void);

// LHLXW 子应用状态追踪（供 auto_test 钩子使用）
// 由 LHLXW.cpp 在 NATIVE_SIMULATION 模式下设置
extern volatile int  hal_lhlxw_option_num;     // -1=不在LHLXW, 0-4=当前选项序号
extern volatile bool hal_lhlxw_subapp_running;  // true=子应用正在运行

#ifdef __cplusplus
}
#endif

#endif // HAL_NATIVE_H