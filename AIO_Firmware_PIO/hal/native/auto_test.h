#ifndef AUTO_TEST_H
#define AUTO_TEST_H

#include <stdint.h>
#include <stdbool.h>

// 初始化自动测试系统（解析命令行参数选择测试目标）
void auto_test_init(int argc, char *argv[]);

// 每帧调用，由 auto_test.cpp 覆盖弱函数 hal_native_auto_test_hook()
void auto_test_tick(void);

// 是否正在运行自动测试
bool auto_test_is_running(void);

// 获取当前测试目标名称
const char *auto_test_get_target(void);

// 系统就绪标志：setup() 完成后由 sim_main.cpp 置 true
extern bool g_system_ready;

// ========================
// 各 APP 测试接口（C++）
// ========================

// 每个 APP 测试函数返回 true 表示测试通过
bool auto_test_app_picture(void);
bool auto_test_app_game_2048(void);
bool auto_test_app_settings(void);
bool auto_test_app_heartbeat(void);
bool auto_test_app_idea_anim(void);
bool auto_test_app_media_player(void);
bool auto_test_app_screen_share(void);
bool auto_test_app_stockmarket(void);
bool auto_test_app_weather(void);
bool auto_test_app_tomato(void);
bool auto_test_app_anniversary(void);
bool auto_test_app_game_snake(void);
bool auto_test_app_bilibili(void);
bool auto_test_app_pc_resource(void);
bool auto_test_app_file_manager(void);
bool auto_test_app_LHLXW(void);

#endif // AUTO_TEST_H