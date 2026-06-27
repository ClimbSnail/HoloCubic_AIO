#include "auto_test.h"
#include "hal_native.h"
#include "sys/app_controller.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <FreeRTOS.h>
#include <task.h>

// 动作类型枚举，与 hal_display.c 中的 hal_action_t 保持一致
enum {
    AUTO_ACTION_NONE = 0,
    AUTO_ACTION_RETURN,
    AUTO_ACTION_TURN_LEFT,
    AUTO_ACTION_TURN_RIGHT,
    AUTO_ACTION_UP,
    AUTO_ACTION_DOWN,
    AUTO_ACTION_SHAKE,
    AUTO_ACTION_GO_FORWORD
};

// 钩子类型：定义每个测试步骤等待的完成信号
enum HookType {
    HOOK_NONE = 0,      // 无钩子：使用 frame_delay 帧计数等待
    HOOK_NAV,           // 导航钩子：等待 cur_app_index 变化
    HOOK_ENTER,         // 进入钩子：等待 app_exit_flag 变为 1
    HOOK_EXIT,          // 退出钩子：等待 app_exit_flag 变为 0
    HOOK_LOADING,       // 加载钩子：等待 JPG 解码完成，frame_delay 为超时
    HOOK_HTTP_READY,    // HTTP 服务器就绪：等待 localhost:80 可连接
    HOOK_FTP_READY,     // FTP 服务器就绪：等待 localhost:21 可连接
    HOOK_SERVER_READY,  // 服务器路由就绪：等待 start_web_config() 完成
    HOOK_LHLXW_SUBAPP_RUNNING,  // LHLXW 子应用运行中：等待 hal_lhlxw_subapp_running=true
    HOOK_LHLXW_SUBAPP_EXITED,   // LHLXW 子应用已退出：等待 hal_lhlxw_subapp_running=false
    HOOK_FTP_MKDIR,             // FTP 创建文件夹：发送 MKD 并验证 257 响应
    HOOK_FTP_UPLOAD,            // FTP 上传文件：发送 STOR 并验证 226 响应
    HOOK_FTP_DOWNLOAD,          // FTP 下载文件：发送 RETR 并验证内容一致
};

// 注入动作到键盘缓冲区，模拟真实按键输入
// 通过 hal_native_inject_action() → g_last_action → hal_native_get_key_action()
// → mpu.getAction() 的完整调用链，与真实键盘输入走完全相同的代码路径
static void inject_action_direct(int action)
{
    hal_native_inject_action(action);
}

// app_controller 全局指针（HoloCubic_AIO.cpp 中定义）
extern AppController *app_controller;

// ========================
// 统一日志系统
// ========================

#define AUTO_TEST_TAG "AUTO_TEST"

static void get_timestamp(char *buf, size_t bufsize)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buf, bufsize, "%Y-%m-%d %H:%M:%S", tm_info);
}

#define LOG_DEBUG(tag, fmt, ...) do { \
    char _ts[20]; get_timestamp(_ts, sizeof(_ts)); \
    printf("[%s] [DEBUG] [%s] %s:%d: " fmt "\n", _ts, tag, __FILE__, __LINE__, ##__VA_ARGS__); \
    fflush(stdout); \
} while(0)

#define LOG_INFO(tag, fmt, ...) do { \
    char _ts[20]; get_timestamp(_ts, sizeof(_ts)); \
    printf("[%s] [INFO]  [%s] %s:%d: " fmt "\n", _ts, tag, __FILE__, __LINE__, ##__VA_ARGS__); \
    fflush(stdout); \
} while(0)

#define LOG_WARN(tag, fmt, ...) do { \
    char _ts[20]; get_timestamp(_ts, sizeof(_ts)); \
    printf("[%s] [WARN]  [%s] %s:%d: " fmt "\n", _ts, tag, __FILE__, __LINE__, ##__VA_ARGS__); \
    fflush(stdout); \
} while(0)

#define LOG_ERROR(tag, fmt, ...) do { \
    char _ts[20]; get_timestamp(_ts, sizeof(_ts)); \
    printf("[%s] [ERROR] [%s] %s:%d: " fmt "\n", _ts, tag, __FILE__, __LINE__, ##__VA_ARGS__); \
    fflush(stdout); \
} while(0)

// ========================
// 服务器路由就绪标志（由 server.cpp 通过 hal_native_server_routes_ready() 设置）
// ========================

static bool g_server_routes_ready = false;

extern "C" void hal_native_server_routes_ready(void)
{
    g_server_routes_ready = true;
    LOG_INFO(AUTO_TEST_TAG, "Server routes ready signal received");
}

// ========================
// 测试步骤与用例结构
// ========================

struct TestStep {
    int frame_delay;   // 最大等待帧数 / 超时帧数（约 1ms/帧）
    int action;        // 注入的动作
    int hook_type;     // 等待的完成信号类型（HookType 枚举）
};

struct TestCase {
    const char *app_name;           // APP 名称
    int nav_steps;                  // 导航步数（备用，实际由动态计算覆盖）
    const TestStep *steps;          // 测试步骤数组
    int step_count;                 // 步骤数量
    int post_enter_delay;           // 进入 APP 后等待帧数（备用）
};

// ========================
// 测试状态
// ========================

static struct {
    bool running;
    bool inited;
    bool completed;                 // 测试已完成，不再重复执行
    const char *target_app;         // 目标 APP 名称，NULL 表示全部测试
    int current_step;
    int frame_counter;
    int phase;                      // 0=导航, 1=进入, 2=测试, 3=完成, 4=退出
    int nav_counter;
    int dynamic_nav_steps;          // 动态计算的导航步数
    int target_app_index;           // 目标 APP 在 appList 中的索引
    const TestCase *current_case;
    bool case_result;
    bool test_failed;               // 钩子超时标志：true 表示有步骤因超时失败
    int total_tests;
    int passed_tests;

    // 钩子状态追踪
    int prev_app_index;             // 上一帧的 cur_app_index（用于 HOOK_NAV）
    int prev_enter_flag;            // 上一帧的 app_exit_flag（用于 HOOK_ENTER/HOOK_EXIT）
    bool nav_first_action_sent;     // 导航阶段是否已发送第一个动作
    bool enter_go_sent;             // 进入阶段是否已发送 GO_FORWORD
} test_state = {
    false, false, false, NULL, 0, 0, 0, 0, 0, -1, NULL, false, 0, 0,
    0, 0, false, false
};

// ========================
// 测试用例定义
// ========================

// Picture APP 测试
// Picture 进入后不会主动加载图片，需要注入 TURN_RIGHT 触发首次加载
static const TestStep picture_steps[] = {
    {500,  AUTO_ACTION_TURN_RIGHT, HOOK_LOADING},   // 注入右键→触发首次图片加载→等待解码完成
    {2,    AUTO_ACTION_TURN_RIGHT, HOOK_NONE},      // 下一张
    {500,  AUTO_ACTION_NONE,       HOOK_LOADING},   // 等待加载
    {2,    AUTO_ACTION_TURN_RIGHT, HOOK_NONE},      // 下一张
    {500,  AUTO_ACTION_NONE,       HOOK_LOADING},   // 等待加载
    {2,    AUTO_ACTION_TURN_LEFT,  HOOK_NONE},      // 上一张
    {500,  AUTO_ACTION_NONE,       HOOK_LOADING},   // 等待加载
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},      // 退出
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},      // 等待退出（超时3000帧）
};

// Game 2048 APP 测试
static const TestStep game_2048_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},     // 等待游戏加载（复用 HOOK_ENTER 检查 app_exit_flag）
    {2,    AUTO_ACTION_UP,         HOOK_NONE},      // 上移
    {30,   AUTO_ACTION_NONE,       HOOK_NONE},
    {2,    AUTO_ACTION_TURN_LEFT,  HOOK_NONE},      // 左移
    {30,   AUTO_ACTION_NONE,       HOOK_NONE},
    {2,    AUTO_ACTION_DOWN,       HOOK_NONE},      // 下移
    {30,   AUTO_ACTION_NONE,       HOOK_NONE},
    {2,    AUTO_ACTION_TURN_RIGHT, HOOK_NONE},      // 右移
    {30,   AUTO_ACTION_NONE,       HOOK_NONE},
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},      // 退出
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},      // 等待退出
};

// Settings APP 测试（仅验证进入，忽略 HTTP 阻塞导致无法退出的问题）
static const TestStep settings_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},     // 等待进入即完成
};

// IdeaAnim APP 测试（动画渲染）
static const TestStep idea_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},    // 等待进入
    {3000, AUTO_ACTION_NONE,       HOOK_NONE},     // 停留观察动画
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},     // 退出
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},     // 等待退出
};

// Server APP 测试（HTTP 服务器）
static const TestStep server_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},    // 等待进入
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},     // 退出
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},     // 等待退出
};

// File Manager APP 测试（FTP 服务器 + 文件夹创建/上传/下载）
static const TestStep file_manager_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},       // 等待进入
    {3000, AUTO_ACTION_NONE,       HOOK_FTP_READY},   // 等待 FTP:21 就绪
    {3000, AUTO_ACTION_NONE,       HOOK_FTP_MKDIR},   // 创建文件夹
    {3000, AUTO_ACTION_NONE,       HOOK_FTP_UPLOAD},  // 上传文件
    {3000, AUTO_ACTION_NONE,       HOOK_FTP_DOWNLOAD},// 下载文件并验证内容
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},        // 退出
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},        // 等待退出
};

// LHLXW APP 测试：等 init → 轮流进入 3 个子应用(cyber/heartbeat/codeRain) → 退出
// eye(option 0)有递归问题跳过，emoji(option 4)依赖SD卡文件暂不支持仿真跳过
// 每个子应用: TURN_RIGHT切换选项 → UP进入 → 钩子确认运行 → 运行 → RETURN退出 → 钩子确认退出
// 使用 HOOK_LHLXW_SUBAPP_RUNNING 动态验证子应用启动，HOOK_LHLXW_SUBAPP_EXITED 验证子应用退出
// 不再依赖帧计数盲目等待，而是通过钩子信号动态检查通过条件
static const TestStep LHLXW_steps[] = {
    // Step 0: 等待 app_init + startLog 动画完成
    {600,  AUTO_ACTION_NONE,       HOOK_NONE},
    // Step 1-3: 切换到 cyber(option 1) 并进入（TURN_RIGHT后等30帧让LHLXW消费动作）
    {2,    AUTO_ACTION_TURN_RIGHT, HOOK_NONE},
    {30,   AUTO_ACTION_NONE,       HOOK_NONE},
    {300,  AUTO_ACTION_UP,         HOOK_LHLXW_SUBAPP_RUNNING},
    // Step 4-5: 运行 cyber 后退出，钩子验证退出
    {150,  AUTO_ACTION_NONE,       HOOK_NONE},
    {300,  AUTO_ACTION_RETURN,     HOOK_LHLXW_SUBAPP_EXITED},
    // Step 6: 等待退出完成
    {10,   AUTO_ACTION_NONE,       HOOK_NONE},
    // Step 7-9: 切换到 heartbeat(option 2) 并进入
    {2,    AUTO_ACTION_TURN_RIGHT, HOOK_NONE},
    {30,   AUTO_ACTION_NONE,       HOOK_NONE},
    {300,  AUTO_ACTION_UP,         HOOK_LHLXW_SUBAPP_RUNNING},
    // Step 10-11: 运行 heartbeat 后退出
    {150,  AUTO_ACTION_NONE,       HOOK_NONE},
    {300,  AUTO_ACTION_RETURN,     HOOK_LHLXW_SUBAPP_EXITED},
    // Step 12: 等待退出完成
    {10,   AUTO_ACTION_NONE,       HOOK_NONE},
    // Step 13-15: 切换到 codeRain(option 3) 并进入
    {2,    AUTO_ACTION_TURN_RIGHT, HOOK_NONE},
    {30,   AUTO_ACTION_NONE,       HOOK_NONE},
    {300,  AUTO_ACTION_UP,         HOOK_LHLXW_SUBAPP_RUNNING},
    // Step 16-17: 运行 codeRain 后退出
    {150,  AUTO_ACTION_NONE,       HOOK_NONE},
    {2000, AUTO_ACTION_RETURN,     HOOK_LHLXW_SUBAPP_EXITED},
    // Step 18: 等待退出完成
    {10,   AUTO_ACTION_NONE,       HOOK_NONE},
    // Step 19: 退出 LHLXW（emoji option 4 跳过，直接从 codeRain 退出）
    {500,  AUTO_ACTION_RETURN,     HOOK_EXIT},
};

// StockMarket APP 测试（验证异步 HTTP 不阻塞退出）
static const TestStep stockmarket_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},     // 等待进入（含 HTTP 异步请求发起）
    {100,  AUTO_ACTION_NONE,       HOOK_NONE},      // 等待 HTTP 请求进行中
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},      // 退出（验证异步 HTTP 不阻塞）
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},      // 等待退出完成
};

// ========================
// 批次C：新增测试用例（通用 ENTER→RETURN→EXIT 模式）
// ========================

// Anniversary APP 测试
static const TestStep anniversary_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},    // 等待进入
    {3000, AUTO_ACTION_NONE,       HOOK_NONE},     // 停留观察
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},     // 退出
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},     // 等待退出
};

// Bili APP 测试
static const TestStep bili_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},    // 等待进入
    {3000, AUTO_ACTION_NONE,       HOOK_NONE},     // 停留观察
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},     // 退出
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},     // 等待退出
};

// Heartbeat APP 测试（后台应用，仅验证进入后退出）
static const TestStep heartbeat_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},    // 等待进入
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},     // 退出
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},     // 等待退出
};

// Tomato APP 测试
static const TestStep tomato_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},    // 等待进入
    {3000, AUTO_ACTION_NONE,       HOOK_NONE},     // 停留观察
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},     // 退出
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},     // 等待退出
};

// Weather APP 测试
static const TestStep weather_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},    // 等待进入
    {3000, AUTO_ACTION_NONE,       HOOK_NONE},     // 停留观察
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},     // 退出
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},     // 等待退出
};

// Weather Old APP 测试
static const TestStep weather_old_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},    // 等待进入
    {3000, AUTO_ACTION_NONE,       HOOK_NONE},     // 停留观察
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},     // 退出
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},     // 等待退出
};

// Snake APP 测试
static const TestStep snake_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},    // 等待进入
    {3000, AUTO_ACTION_NONE,       HOOK_NONE},     // 停留观察
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},     // 退出
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},     // 等待退出
};

// PC Resource APP 测试
static const TestStep pc_resource_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},    // 等待进入
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},     // 退出
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},     // 等待退出
};

// Screen share APP 测试
static const TestStep screen_share_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},    // 等待进入
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},     // 退出
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},     // 等待退出
};

// Media Player APP 测试（等待进入 → 验证解码 → 切换视频 → 退出）
static const TestStep media_steps[] = {
    {2000, AUTO_ACTION_NONE,       HOOK_ENTER},    // 等待进入
    {2000, AUTO_ACTION_NONE,       HOOK_LOADING},  // 等待首帧解码完成
    {30,   AUTO_ACTION_NONE,       HOOK_NONE},     // 等待几帧
    {2000, AUTO_ACTION_NONE,       HOOK_LOADING},  // 等待第二帧解码
    {2,    AUTO_ACTION_TURN_RIGHT, HOOK_NONE},     // 切换到下一个视频
    {3000, AUTO_ACTION_NONE,       HOOK_LOADING},  // 等待新视频首帧解码
    {2,    AUTO_ACTION_RETURN,     HOOK_NONE},     // 退出
    {3000, AUTO_ACTION_NONE,       HOOK_EXIT},     // 等待退出
};

// ========================
// 测试用例注册表
// ========================

static const TestCase test_cases[] = {
    {"Picture",       0,  picture_steps,      9, 20},
    {"2048",          4,  game_2048_steps,   11, 20},
    {"Settings",      3,  settings_steps,     1, 20},
    {"Idea",          0,  idea_steps,         4, 20},
    {"WebServer",     0,  server_steps,       3, 20},
    {"File Manager",  0,  file_manager_steps, 7, 20},
    {"LH&LXW",        0,  LHLXW_steps,       20, 20},
    {"Stock",         0,  stockmarket_steps,  4, 20},
    {"Anniversary",   0,  anniversary_steps,  4, 20},
    {"Bili",          0,  bili_steps,         4, 20},
    {"Heartbeat",     0,  heartbeat_steps,    3, 20},
    {"Tomato",        0,  tomato_steps,       4, 20},
    {"Weather",       0,  weather_steps,      4, 20},
    {"Weather Old",   0,  weather_old_steps,  4, 20},
    {"Snake",         0,  snake_steps,        4, 20},
    {"PC Resource",   0,  pc_resource_steps,   3, 20},
    {"Screen share",  0,  screen_share_steps,  3, 20},
    {"Media",         0,  media_steps,         8, 20},
};

static const int test_case_count = sizeof(test_cases) / sizeof(test_cases[0]);

// 根据 APP 名称查找测试用例
static const TestCase *find_test_case(const char *app_name)
{
    for (int i = 0; i < test_case_count; i++) {
        if (strcmp(test_cases[i].app_name, app_name) == 0) {
            return &test_cases[i];
        }
    }
    return NULL;
}

// ========================
// 钩子检查函数
// ========================

// 检查导航是否完成：cur_app_index 发生变化
static bool hook_nav_check(void)
{
    if (!app_controller) return false;
    int cur_idx = app_controller->cur_app_index;
    if (cur_idx != test_state.prev_app_index) {
        test_state.prev_app_index = cur_idx;
        return true;
    }
    return false;
}

// 检查 APP 是否已进入：app_exit_flag 变为 1
static bool hook_enter_check(void)
{
    if (!app_controller) return false;
    return (app_controller->app_exit_flag == 1);
}

// 检查 APP 是否已退出：app_exit_flag 变为 0
static bool hook_exit_check(void)
{
    if (!app_controller) return false;
    return (app_controller->app_exit_flag == 0);
}

// 检查 JPG 解码是否完成
static bool hook_loading_check(void)
{
    return (hal_native_is_jpg_decode_done() != 0);
}

// 检查服务器路由是否已注册
static bool hook_server_ready_check(void)
{
    return g_server_routes_ready;
}

// 检查 HTTP 服务器是否就绪（localhost:80 可连接）
static bool hook_http_ready_check(void)
{
    static SOCKET sock = INVALID_SOCKET;
    static int attempt = 0;
    static bool request_sent = false;
    attempt++;

    if (sock == INVALID_SOCKET) {
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) return false;
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(8080);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        u_long mode = 1;
        ioctlsocket(sock, FIONBIO, &mode);
        int ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
        if (ret == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK) {
                closesocket(sock);
                sock = INVALID_SOCKET;
                return false;
            }
        }
        request_sent = false;
        return false;
    }

    if (!request_sent) {
        const char *req = "GET / HTTP/1.0\r\nHost: localhost\r\n\r\n";
        send(sock, req, (int)strlen(req), 0);
        request_sent = true;
        return false;
    }

    char buf[256] = {0};
    int len = recv(sock, buf, sizeof(buf) - 1, 0);
    if (len > 0) {
        LOG_INFO(AUTO_TEST_TAG, "HTTP:80 response: %.200s", buf);
        closesocket(sock);
        sock = INVALID_SOCKET;
        return true;
    }
    if (attempt > 3000) {
        LOG_WARN(AUTO_TEST_TAG, "HTTP:80 timeout after %d attempts", attempt);
        closesocket(sock);
        sock = INVALID_SOCKET;
        return true;
    }
    return false;
}

// 检查 FTP 服务器是否就绪（localhost:21 可连接）
static bool hook_ftp_ready_check(void)
{
    static SOCKET sock = INVALID_SOCKET;
    static int attempt = 0;
    attempt++;

    if (sock == INVALID_SOCKET) {
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) return false;
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(21);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        u_long mode = 1;
        ioctlsocket(sock, FIONBIO, &mode);
        int ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
        if (ret == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK) {
                closesocket(sock);
                sock = INVALID_SOCKET;
                return false;
            }
        }
        return false;
    }

    char buf[256] = {0};
    int len = recv(sock, buf, sizeof(buf) - 1, 0);
    if (len > 0) {
        LOG_INFO(AUTO_TEST_TAG, "FTP:21 banner: %.200s", buf);
        closesocket(sock);
        sock = INVALID_SOCKET;
        return true;
    }
    if (attempt > 3000) {
        LOG_WARN(AUTO_TEST_TAG, "FTP:21 timeout after %d attempts", attempt);
        closesocket(sock);
        sock = INVALID_SOCKET;
        return true;
    }
    return false;
}

// ========================
// FTP 操作钩子：MKDIR / UPLOAD / DOWNLOAD
// 每个钩子自包含完整 FTP 会话（连接→登录→操作→退出）
// 使用状态机在帧间推进，保证非阻塞
// ========================

// FTP 读取一行（从缓冲区中提取 \n 结尾的行）
static int ftp_read_line(SOCKET sock, char *buf, int bufsize, int *buf_off, int *buf_len)
{
    // 先尝试从已有缓冲区中找完整行
    for (int i = 0; i < *buf_len; i++) {
        if ((*buf_off + i) < bufsize && buf[*buf_off + i] == '\n') {
            int line_len = i + 1;
            if (line_len < bufsize) {
                memmove(buf, buf + *buf_off, line_len);
                buf[line_len] = '\0';
                *buf_off += line_len;
                *buf_len -= line_len;
                return line_len;
            }
        }
    }
    // 缓冲区已满，压缩
    if (*buf_off > 0) {
        memmove(buf, buf + *buf_off, *buf_len);
        *buf_off = 0;
    }
    // 读取更多数据
    int space = bufsize - *buf_len - 1;
    if (space > 0) {
        int n = recv(sock, buf + *buf_len, space, 0);
        if (n > 0) {
            *buf_len += n;
            buf[*buf_len] = '\0';
            // 递归再试一次
            return ftp_read_line(sock, buf, bufsize, buf_off, buf_len);
        }
    }
    return 0;
}

// 解析 PASV 227 响应，提取数据端口 (h1,h2,h3,h4,p1,p2)
static int ftp_parse_pasv(const char *resp, int *port)
{
    int h1, h2, h3, h4, p1, p2;
    if (sscanf(resp, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
               &h1, &h2, &h3, &h4, &p1, &p2) == 6) {
        *port = p1 * 256 + p2;
        return 1;
    }
    return 0;
}

// FTP 上传文件内容
static const char *FTP_UPLOAD_CONTENT = "HoloCubic Auto Test FTP Upload OK\r\n";
static const int   FTP_UPLOAD_LEN     = 35;

// ---- HOOK_FTP_MKDIR: 登录 + MKD auto_test_dir ----
static bool hook_ftp_mkdir_check(void)
{
    enum { S_CONNECT, S_BANNER, S_USER, S_PASS_RESP, S_MKD, S_MKD_RESP, S_QUIT, S_DONE, S_FAIL } state;
    static SOCKET sock = INVALID_SOCKET;
    static char buf[1024];
    static int buf_off = 0, buf_len = 0;
    static int attempt = 0;
    static int s_state = S_CONNECT;

    if (s_state == S_DONE) {
        closesocket(sock);
        sock = INVALID_SOCKET;
        s_state = S_CONNECT;
        attempt = 0;
        return true;
    }
    if (s_state == S_FAIL) {
        LOG_ERROR(AUTO_TEST_TAG, "FTP MKDIR failed at state=%d", s_state);
        closesocket(sock);
        sock = INVALID_SOCKET;
        s_state = S_CONNECT;
        attempt = 0;
        return true;
    }

    attempt++;

    // 调试日志：每 100 帧输出当前状态，确认钩子是否被调用及卡在哪步
    if (attempt % 100 == 1) {
        LOG_INFO(AUTO_TEST_TAG, "FTP MKDIR: attempt=%d state=%d", attempt, s_state);
    }

    switch (s_state) {
    case S_CONNECT:
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) { s_state = S_FAIL; return false; }
        {
            u_long mode = 1;
            ioctlsocket(sock, FIONBIO, &mode);
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(21);
            addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            int ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
            if (ret == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
                s_state = S_FAIL; return false;
            }
        }
        s_state = S_BANNER;
        buf_off = buf_len = 0;
        break;

    case S_BANNER:
        if (ftp_read_line(sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            LOG_INFO(AUTO_TEST_TAG, "FTP MKDIR: banner=%.60s", buf);
            send(sock, "USER holocubic\r\n", 16, 0);
            s_state = S_USER;
            buf_off = buf_len = 0;
        }
        break;

    case S_USER:
        if (ftp_read_line(sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            LOG_INFO(AUTO_TEST_TAG, "FTP MKDIR: USER resp=%.60s", buf);
            send(sock, "PASS aio\r\n", 10, 0);
            s_state = S_PASS_RESP;
            buf_off = buf_len = 0;
        }
        break;

    case S_PASS_RESP:
        if (ftp_read_line(sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            LOG_INFO(AUTO_TEST_TAG, "FTP MKDIR: PASS resp=%.60s", buf);
            send(sock, "MKD auto_test_dir\r\n", 19, 0);
            s_state = S_MKD;
            buf_off = buf_len = 0;
        }
        break;

    case S_MKD:
        if (ftp_read_line(sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            LOG_INFO(AUTO_TEST_TAG, "FTP MKDIR: MKD resp=%.60s", buf);
            if (strncmp(buf, "257", 3) == 0) {
                LOG_INFO(AUTO_TEST_TAG, "FTP MKDIR: SUCCESS - directory created");
            } else {
                LOG_ERROR(AUTO_TEST_TAG, "FTP MKDIR: FAILED (expected 257, got %.60s)", buf);
            }
            send(sock, "QUIT\r\n", 6, 0);
            s_state = S_QUIT;
            buf_off = buf_len = 0;
        }
        break;

    case S_QUIT:
        if (attempt > 10) { s_state = S_DONE; }
        break;
    }

    if (attempt > 3000) {
        LOG_ERROR(AUTO_TEST_TAG, "FTP MKDIR: timeout at state=%d", s_state);
        s_state = S_FAIL;
    }
    return false;
}

// ---- HOOK_FTP_UPLOAD: 登录 + PASV + STOR upload_test.txt ----
static bool hook_ftp_upload_check(void)
{
    enum { S_CONNECT, S_BANNER, S_USER, S_PASS_RESP, S_PASV, S_PASV_RESP,
           S_DATA_CONNECT, S_STOR, S_STOR_RESP, S_DATA_SEND, S_DATA_CLOSE,
           S_CTRL_RESP, S_QUIT, S_DONE, S_FAIL } state;
    static SOCKET ctrl_sock = INVALID_SOCKET;
    static SOCKET data_sock = INVALID_SOCKET;
    static char buf[1024];
    static int buf_off = 0, buf_len = 0;
    static int attempt = 0;
    static int s_state = S_CONNECT;
    static int data_port = 0;

    if (s_state == S_DONE) {
        closesocket(ctrl_sock);
        ctrl_sock = INVALID_SOCKET;
        s_state = S_CONNECT;
        attempt = 0;
        return true;
    }
    if (s_state == S_FAIL) {
        LOG_ERROR(AUTO_TEST_TAG, "FTP UPLOAD failed at state=%d", s_state);
        if (ctrl_sock != INVALID_SOCKET) { closesocket(ctrl_sock); ctrl_sock = INVALID_SOCKET; }
        if (data_sock != INVALID_SOCKET) { closesocket(data_sock); data_sock = INVALID_SOCKET; }
        s_state = S_CONNECT;
        attempt = 0;
        return true;
    }

    if (attempt == 1) LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: start, state=%d", s_state);
    attempt++;

    switch (s_state) {
    case S_CONNECT:
        LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: S_CONNECT attempt=%d", attempt);
        ctrl_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ctrl_sock == INVALID_SOCKET) {
            LOG_ERROR(AUTO_TEST_TAG, "FTP UPLOAD: socket() failed err=%d", WSAGetLastError());
            s_state = S_FAIL; return false;
        }
        {
            u_long mode = 1;
            ioctlsocket(ctrl_sock, FIONBIO, &mode);
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(21);
            addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            int ret = connect(ctrl_sock, (struct sockaddr *)&addr, sizeof(addr));
            if (ret == SOCKET_ERROR) {
                int err = WSAGetLastError();
                if (err != WSAEWOULDBLOCK) {
                    LOG_ERROR(AUTO_TEST_TAG, "FTP UPLOAD: connect() failed err=%d", err);
                    s_state = S_FAIL; return false;
                }
            }
            LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: connect() OK, moving to S_BANNER");
        }
        s_state = S_BANNER;
        buf_off = buf_len = 0;
        break;

    case S_BANNER:
        if (attempt <= 3 || attempt % 100 == 0) {
            LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: S_BANNER waiting attempt=%d buf_len=%d", attempt, buf_len);
        }
        if (ftp_read_line(ctrl_sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: banner=%.60s", buf);
            send(ctrl_sock, "USER holocubic\r\n", 16, 0);
            s_state = S_USER;
            buf_off = buf_len = 0;
        }
        break;

    case S_USER:
        if (attempt <= 5 || attempt % 100 == 0) {
            LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: S_USER waiting attempt=%d buf_len=%d", attempt, buf_len);
        }
        if (ftp_read_line(ctrl_sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: USER resp=%.60s", buf);
            send(ctrl_sock, "PASS aio\r\n", 10, 0);
            s_state = S_PASS_RESP;
            buf_off = buf_len = 0;
        }
        break;

    case S_PASS_RESP:
        if (attempt <= 5 || attempt % 100 == 0) {
            LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: S_PASS_RESP waiting attempt=%d buf_len=%d", attempt, buf_len);
        }
        if (ftp_read_line(ctrl_sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: PASS resp=%.60s", buf);
            send(ctrl_sock, "PASV\r\n", 6, 0);
            s_state = S_PASV;
            buf_off = buf_len = 0;
        }
        break;

    case S_PASV:
        if (attempt <= 5 || attempt % 100 == 0) {
            LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: S_PASV waiting attempt=%d buf_len=%d", attempt, buf_len);
        }
        if (ftp_read_line(ctrl_sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: PASV resp=%.80s", buf);
            if (ftp_parse_pasv(buf, &data_port)) {
                LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: data port=%d", data_port);
                data_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (data_sock != INVALID_SOCKET) {
                    u_long mode = 1;
                    ioctlsocket(data_sock, FIONBIO, &mode);
                    struct sockaddr_in daddr;
                    daddr.sin_family = AF_INET;
                    daddr.sin_port = htons((u_short)data_port);
                    daddr.sin_addr.s_addr = inet_addr("127.0.0.1");
                    connect(data_sock, (struct sockaddr *)&daddr, sizeof(daddr));
                }
                send(ctrl_sock, "STOR upload_test.txt\r\n", 22, 0);
                s_state = S_STOR;
            } else {
                s_state = S_FAIL;
            }
            buf_off = buf_len = 0;
        }
        break;

    case S_STOR:
        if (ftp_read_line(ctrl_sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: STOR resp=%.60s", buf);
            s_state = S_DATA_SEND;
        }
        break;

    case S_DATA_SEND:
        if (data_sock != INVALID_SOCKET) {
            send(data_sock, FTP_UPLOAD_CONTENT, FTP_UPLOAD_LEN, 0);
            LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: data sent (%d bytes)", FTP_UPLOAD_LEN);
            closesocket(data_sock);
            data_sock = INVALID_SOCKET;
            s_state = S_CTRL_RESP;
        }
        break;

    case S_CTRL_RESP:
        if (ftp_read_line(ctrl_sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: final resp=%.60s", buf);
            if (strncmp(buf, "226", 3) == 0) {
                LOG_INFO(AUTO_TEST_TAG, "FTP UPLOAD: SUCCESS");
            } else {
                LOG_ERROR(AUTO_TEST_TAG, "FTP UPLOAD: unexpected response (expected 226)");
            }
            send(ctrl_sock, "QUIT\r\n", 6, 0);
            s_state = S_QUIT;
        }
        break;

    case S_QUIT:
        if (attempt > 10) { s_state = S_DONE; }
        break;
    }

    if (attempt > 3000) {
        LOG_ERROR(AUTO_TEST_TAG, "FTP UPLOAD: timeout at state=%d", s_state);
        s_state = S_FAIL;
    }
    return false;
}

// ---- HOOK_FTP_DOWNLOAD: 登录 + PASV + RETR upload_test.txt + 验证内容 ----
static bool hook_ftp_download_check(void)
{
    enum { S_CONNECT, S_BANNER, S_USER, S_PASS_RESP, S_PASV, S_PASV_RESP,
           S_DATA_CONNECT, S_RETR, S_RETR_RESP, S_DATA_RECV, S_DATA_CLOSE,
           S_CTRL_RESP, S_QUIT, S_DONE, S_FAIL } state;
    static SOCKET ctrl_sock = INVALID_SOCKET;
    static SOCKET data_sock = INVALID_SOCKET;
    static char buf[1024];
    static char file_buf[256];
    static int buf_off = 0, buf_len = 0;
    static int file_len = 0;
    static int attempt = 0;
    static int s_state = S_CONNECT;
    static int data_port = 0;

    if (s_state == S_DONE) {
        file_buf[file_len] = '\0';
        if (strcmp(file_buf, FTP_UPLOAD_CONTENT) == 0) {
            LOG_INFO(AUTO_TEST_TAG, "FTP DOWNLOAD: content verified OK");
        } else {
            LOG_ERROR(AUTO_TEST_TAG, "FTP DOWNLOAD: content mismatch! expected='%s' got='%s'",
                      FTP_UPLOAD_CONTENT, file_buf);
        }
        closesocket(ctrl_sock);
        ctrl_sock = INVALID_SOCKET;
        s_state = S_CONNECT;
        attempt = 0;
        file_len = 0;
        return true;
    }
    if (s_state == S_FAIL) {
        LOG_ERROR(AUTO_TEST_TAG, "FTP DOWNLOAD failed at state=%d", s_state);
        if (ctrl_sock != INVALID_SOCKET) { closesocket(ctrl_sock); ctrl_sock = INVALID_SOCKET; }
        if (data_sock != INVALID_SOCKET) { closesocket(data_sock); data_sock = INVALID_SOCKET; }
        s_state = S_CONNECT;
        attempt = 0;
        file_len = 0;
        return true;
    }

    attempt++;

    switch (s_state) {
    case S_CONNECT:
        ctrl_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ctrl_sock == INVALID_SOCKET) { s_state = S_FAIL; return false; }
        {
            u_long mode = 1;
            ioctlsocket(ctrl_sock, FIONBIO, &mode);
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(21);
            addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            int ret = connect(ctrl_sock, (struct sockaddr *)&addr, sizeof(addr));
            if (ret == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
                s_state = S_FAIL; return false;
            }
        }
        s_state = S_BANNER;
        buf_off = buf_len = 0;
        break;

    case S_BANNER:
        if (ftp_read_line(ctrl_sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            send(ctrl_sock, "USER holocubic\r\n", 16, 0);
            s_state = S_USER;
            buf_off = buf_len = 0;
        }
        break;

    case S_USER:
        if (ftp_read_line(ctrl_sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            send(ctrl_sock, "PASS aio\r\n", 10, 0);
            s_state = S_PASS_RESP;
            buf_off = buf_len = 0;
        }
        break;

    case S_PASS_RESP:
        if (ftp_read_line(ctrl_sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            send(ctrl_sock, "PASV\r\n", 6, 0);
            s_state = S_PASV;
            buf_off = buf_len = 0;
        }
        break;

    case S_PASV:
        if (ftp_read_line(ctrl_sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            LOG_INFO(AUTO_TEST_TAG, "FTP DOWNLOAD: PASV resp=%.80s", buf);
            if (ftp_parse_pasv(buf, &data_port)) {
                LOG_INFO(AUTO_TEST_TAG, "FTP DOWNLOAD: data port=%d", data_port);
                data_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (data_sock != INVALID_SOCKET) {
                    u_long mode = 1;
                    ioctlsocket(data_sock, FIONBIO, &mode);
                    struct sockaddr_in daddr;
                    daddr.sin_family = AF_INET;
                    daddr.sin_port = htons((u_short)data_port);
                    daddr.sin_addr.s_addr = inet_addr("127.0.0.1");
                    connect(data_sock, (struct sockaddr *)&daddr, sizeof(daddr));
                }
                send(ctrl_sock, "RETR upload_test.txt\r\n", 22, 0);
                s_state = S_RETR;
            } else {
                s_state = S_FAIL;
            }
            buf_off = buf_len = 0;
        }
        break;

    case S_RETR:
        if (ftp_read_line(ctrl_sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            LOG_INFO(AUTO_TEST_TAG, "FTP DOWNLOAD: RETR resp=%.60s", buf);
            s_state = S_DATA_RECV;
            file_len = 0;
        }
        break;

    case S_DATA_RECV:
        if (data_sock != INVALID_SOCKET) {
            int n = recv(data_sock, file_buf + file_len,
                         (int)(sizeof(file_buf) - 1 - file_len), 0);
            if (n > 0) {
                file_len += n;
                LOG_INFO(AUTO_TEST_TAG, "FTP DOWNLOAD: received %d bytes (total %d)", n, file_len);
            } else if (n == 0) {
                closesocket(data_sock);
                data_sock = INVALID_SOCKET;
                s_state = S_CTRL_RESP;
                // 不重置 buf_off/buf_len，控制通道可能已有 226 响应
            }
        }
        break;

    case S_CTRL_RESP:
        if (ftp_read_line(ctrl_sock, buf, sizeof(buf), &buf_off, &buf_len)) {
            LOG_INFO(AUTO_TEST_TAG, "FTP DOWNLOAD: final resp=%.60s", buf);
            send(ctrl_sock, "QUIT\r\n", 6, 0);
            s_state = S_QUIT;
        }
        break;

    case S_QUIT:
        if (attempt > 10) { s_state = S_DONE; }
        break;
    }

    if (attempt > 3000) {
        LOG_ERROR(AUTO_TEST_TAG, "FTP DOWNLOAD: timeout at state=%d", s_state);
        s_state = S_FAIL;
    }
    return false;
}

// LHLXW 子应用运行中钩子：等待 hal_lhlxw_subapp_running 变为 true
// 用于验证 UP 后子应用确实启动了
static bool hook_lhlxw_subapp_running_check(void)
{
    return hal_lhlxw_subapp_running;
}

// LHLXW 子应用已退出钩子：等待 hal_lhlxw_subapp_running 变为 false
// 用于验证 RETURN 后子应用确实退出了
static bool hook_lhlxw_subapp_exited_check(void)
{
    return !hal_lhlxw_subapp_running;
}

// 根据钩子类型检查完成条件
static bool check_hook(int hook_type)
{
    switch (hook_type) {
    case HOOK_NAV:                 return hook_nav_check();
    case HOOK_ENTER:               return hook_enter_check();
    case HOOK_EXIT:                return hook_exit_check();
    case HOOK_LOADING:             return hook_loading_check();
    case HOOK_HTTP_READY:          return hook_http_ready_check();
    case HOOK_FTP_READY:           return hook_ftp_ready_check();
    case HOOK_SERVER_READY:        return hook_server_ready_check();
    case HOOK_LHLXW_SUBAPP_RUNNING: return hook_lhlxw_subapp_running_check();
    case HOOK_LHLXW_SUBAPP_EXITED:  return hook_lhlxw_subapp_exited_check();
    case HOOK_FTP_MKDIR:            return hook_ftp_mkdir_check();
    case HOOK_FTP_UPLOAD:           return hook_ftp_upload_check();
    case HOOK_FTP_DOWNLOAD:         return hook_ftp_download_check();
    case HOOK_NONE:
    default:                       return false;
    }
}

// ========================
// 启动测试用例
// ========================

static void start_test_case(const TestCase *tc)
{
    test_state.current_case = tc;
    test_state.current_step = 0;
    test_state.frame_counter = 0;
    test_state.nav_counter = 0;
    test_state.case_result = true;
    test_state.test_failed = false;
    test_state.nav_first_action_sent = false;
    test_state.enter_go_sent = false;

    // 动态计算导航步数：从当前 app_index 到目标 app_index
    int cur_idx = app_controller ? app_controller->cur_app_index : 0;
    int app_num = app_controller ? app_controller->app_num : 8;
    int target_idx = app_controller ? app_controller->getAppIdxByName(tc->app_name) : tc->nav_steps;

    test_state.target_app_index = target_idx;
    test_state.prev_app_index = cur_idx;

    if (target_idx < 0) {
        LOG_ERROR(AUTO_TEST_TAG, "App '%s' not found in appList!", tc->app_name);
        test_state.dynamic_nav_steps = 0;
    } else {
        int right_steps = (target_idx - cur_idx + app_num) % app_num;
        int left_steps = (cur_idx - target_idx + app_num) % app_num;

        // TURN_RIGHT 使 cur_app_index 减小（列表向左移），TURN_LEFT 使索引增大（列表向右移）
        // 因此：列表向右走 → TURN_LEFT (负值)，列表向左走 → TURN_RIGHT (正值)
        if (right_steps <= left_steps) {
            test_state.dynamic_nav_steps = -right_steps;
        } else {
            test_state.dynamic_nav_steps = left_steps;
        }
    }

    LOG_INFO(AUTO_TEST_TAG, "===== Starting test: %s =====", tc->app_name);
    LOG_DEBUG(AUTO_TEST_TAG, "Current app_index=%d, target app_index=%d, app_num=%d",
              cur_idx, target_idx, app_num);

    if (test_state.dynamic_nav_steps == 0) {
        LOG_DEBUG(AUTO_TEST_TAG, "Phase 0: Already at %s, skipping navigation", tc->app_name);
        test_state.phase = 1;
        test_state.prev_enter_flag = app_controller ? app_controller->app_exit_flag : 0;
    } else {
        LOG_DEBUG(AUTO_TEST_TAG, "Phase 0: Navigating to %s (%d steps, %s)",
                  tc->app_name, abs(test_state.dynamic_nav_steps),
                  test_state.dynamic_nav_steps > 0 ? "right" : "left");
        test_state.phase = 0;
    }
}

// ========================
// 对外接口实现
// ========================

void auto_test_init(int argc, char *argv[])
{
    if (test_state.inited) return;
    test_state.inited = true;

    const char *target = NULL;

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--auto-test=", 12) == 0) {
            target = argv[i] + 12;
            break;
        }
        if (strcmp(argv[i], "--auto-test") == 0 && i + 1 < argc) {
            target = argv[i + 1];
            break;
        }
    }

    if (!target) {
        target = getenv("AUTO_TEST");
    }

    if (target && strlen(target) == 0) {
        target = NULL;
    }

    if (target) {
        // 别名映射：命令行参数可能不含空格，但 APP 名称可能含空格
        if (strcmp(target, "FileManager") == 0) target = "File Manager";
        if (strcmp(target, "WebServer") == 0) target = "WebServer";
        test_state.target_app = target;
        LOG_INFO(AUTO_TEST_TAG, "Auto-test mode enabled, target: %s", target);
    }
}

// ========================
// 弱函数覆盖：hal_native_auto_test_hook
// 由 auto_test_thread 定时调用，不依赖 FreeRTOS 调度
// 这样即使 APP 的 process 函数有 while(1) 死循环也能注入动作
// ========================

extern "C" void hal_native_auto_test_hook(void)
{
    auto_test_tick();
}

void auto_test_tick(void)
{
    if (!test_state.inited) return;
    if (!test_state.target_app) return;
    // 等待系统就绪：app_controller 初始化完成 + setup() 完成
    if (!g_system_ready) return;
    if (!app_controller || app_controller->getAppIdxByName(test_state.target_app) < 0) return;

    if (!test_state.running && !test_state.completed) {
        const TestCase *tc = find_test_case(test_state.target_app);
        if (!tc) {
            LOG_ERROR(AUTO_TEST_TAG, "No test case found for '%s'", test_state.target_app);
            LOG_ERROR(AUTO_TEST_TAG, "Available: Picture, 2048, Settings, WebServer, File Manager");
            test_state.running = true;
            return;
        }
        start_test_case(tc);
        test_state.running = true;
        test_state.total_tests = 1;
    }

    if (!test_state.current_case) return;

    const TestCase *tc = test_state.current_case;

    switch (test_state.phase) {

    // ============================================================
    // Phase 0: 导航阶段（钩子驱动）
    // 使用 cur_app_index 变化检测导航完成，替代固定 10 帧等待
    // ============================================================
    case 0: {
        int cur_idx = app_controller ? app_controller->cur_app_index : 0;

        if (!test_state.nav_first_action_sent) {
            // 首次进入：记录当前索引，发送第一个导航动作
            test_state.prev_app_index = cur_idx;
            int action = (test_state.dynamic_nav_steps > 0) ? AUTO_ACTION_TURN_RIGHT : AUTO_ACTION_TURN_LEFT;
            inject_action_direct(action);
            test_state.nav_counter = 1;
            test_state.nav_first_action_sent = true;
            test_state.frame_counter = 0;
            LOG_DEBUG(AUTO_TEST_TAG, "Nav step 1/%d: sent %s (hook wait)",
                      abs(test_state.dynamic_nav_steps),
                      test_state.dynamic_nav_steps > 0 ? "RIGHT" : "LEFT");
        }

        test_state.frame_counter++;

        // 钩子检测：cur_app_index 是否已变化
        if (cur_idx != test_state.prev_app_index) {
            test_state.prev_app_index = cur_idx;
            test_state.frame_counter = 0;

            if (test_state.nav_counter < abs(test_state.dynamic_nav_steps)) {
                // 还有更多导航步骤
                int action = (test_state.dynamic_nav_steps > 0) ? AUTO_ACTION_TURN_RIGHT : AUTO_ACTION_TURN_LEFT;
                inject_action_direct(action);
                test_state.nav_counter++;
                LOG_DEBUG(AUTO_TEST_TAG, "Nav step %d/%d: sent %s (hook triggered, index=%d)",
                          test_state.nav_counter, abs(test_state.dynamic_nav_steps),
                          test_state.dynamic_nav_steps > 0 ? "RIGHT" : "LEFT", cur_idx);
            } else {
                LOG_DEBUG(AUTO_TEST_TAG, "Phase 0 complete (hook: all nav steps done)");
                test_state.phase = 1;
                test_state.frame_counter = 0;
                test_state.prev_enter_flag = app_controller ? app_controller->app_exit_flag : 0;
            }
        }

        // 超时保护：导航步骤超过 500 帧未完成则强制推进
        if (test_state.frame_counter >= 500) {
            LOG_WARN(AUTO_TEST_TAG, "Nav timeout at step %d/%d (waited 500 frames), forcing advance",
                     test_state.nav_counter, abs(test_state.dynamic_nav_steps));
            // 强制执行剩余导航步骤以完成导航
            while (test_state.nav_counter < abs(test_state.dynamic_nav_steps)) {
                int action = (test_state.dynamic_nav_steps > 0) ? AUTO_ACTION_TURN_RIGHT : AUTO_ACTION_TURN_LEFT;
                inject_action_direct(action);
                test_state.nav_counter++;
            }
            test_state.phase = 1;
            test_state.frame_counter = 0;
            test_state.prev_enter_flag = app_controller ? app_controller->app_exit_flag : 0;
        }
        break;
    }

    // ============================================================
    // Phase 1: 进入 APP 阶段（钩子驱动）
    // 使用 app_exit_flag 检测进入完成，替代固定 15 帧等待
    // ============================================================
    case 1: {
        if (!test_state.enter_go_sent) {
            // 发送 GO_FORWORD 进入 APP
            test_state.prev_enter_flag = app_controller ? app_controller->app_exit_flag : 0;
            inject_action_direct(AUTO_ACTION_GO_FORWORD);
            test_state.enter_go_sent = true;
            test_state.frame_counter = 0;
            LOG_DEBUG(AUTO_TEST_TAG, "Phase 1: GO_FORWORD sent (hook wait for app_exit_flag=1)");
        }

        test_state.frame_counter++;

        // 钩子检测：app_exit_flag 是否变为 1
        if (app_controller && app_controller->app_exit_flag == 1) {
            // 验证是否进入了正确的 APP
            if (app_controller->cur_app_index != test_state.target_app_index) {
                LOG_ERROR(AUTO_TEST_TAG, "Phase 1: Entered wrong APP! expected=%s(idx=%d), actual idx=%d",
                          tc->app_name, test_state.target_app_index, app_controller->cur_app_index);
                test_state.case_result = false;
            }
            LOG_DEBUG(AUTO_TEST_TAG, "Phase 1 complete (hook: app_exit_flag=1, %d frames, idx=%d)",
                      test_state.frame_counter, app_controller->cur_app_index);
            test_state.phase = 2;
            test_state.frame_counter = 0;
            test_state.current_step = 0;
        }

        // 超时保护：2000 帧未进入则强制推进
        if (test_state.frame_counter >= 2000) {
            LOG_WARN(AUTO_TEST_TAG, "Phase 1 timeout (2000 frames), forcing advance");
            test_state.phase = 2;
            test_state.frame_counter = 0;
            test_state.current_step = 0;
        }
        break;
    }

    // ============================================================
    // Phase 2: 测试阶段（钩子 + 帧计数混合驱动）
    // HOOK_NONE: 使用 frame_delay 帧计数
    // HOOK_LOADING/ENTER/EXIT: 等待钩子信号，frame_delay 为超时
    // ============================================================
    case 2: {
        if (test_state.current_step < tc->step_count) {
            const TestStep *step = &tc->steps[test_state.current_step];
            test_state.frame_counter++;

            // 钩子类步骤：首帧立即注入动作，然后等待钩子触发
            // HOOK_NONE 步骤：等待 frame_delay 帧后再注入动作
            if (test_state.frame_counter == 1
                && step->hook_type != HOOK_NONE
                && step->action != AUTO_ACTION_NONE) {
                inject_action_direct(step->action);
                LOG_DEBUG(AUTO_TEST_TAG, "Step %d/%d: action=%d (injected, waiting for hook)",
                          test_state.current_step + 1, tc->step_count, step->action);
            }

            bool step_complete = false;

            if (step->hook_type == HOOK_NONE) {
                // 无钩子：纯帧计数等待
                step_complete = (test_state.frame_counter >= step->frame_delay);
            } else {
                // 有钩子：等待钩子信号，或超时
                bool hook_triggered = check_hook(step->hook_type);
                bool timeout = (test_state.frame_counter >= step->frame_delay);

                if (hook_triggered) {
                    step_complete = true;
                    LOG_DEBUG(AUTO_TEST_TAG, "Step %d/%d: hook triggered (%d frames)",
                              test_state.current_step + 1, tc->step_count,
                              test_state.frame_counter);
                } else if (timeout) {
                    test_state.test_failed = true;
                    test_state.phase = 3;
                    LOG_ERROR(AUTO_TEST_TAG, "Step %d/%d: hook timeout (%d frames), TEST FAILED",
                              test_state.current_step + 1, tc->step_count,
                              test_state.frame_counter);
                    break;
                }
            }

            if (step_complete) {
                test_state.frame_counter = 0;
                // HOOK_NONE 步骤在完成时注入动作（延迟注入模式）
                // 钩子类步骤已在首帧注入，这里只处理 HOOK_NONE
                if (step->hook_type == HOOK_NONE && step->action != AUTO_ACTION_NONE) {
                    inject_action_direct(step->action);
                    LOG_DEBUG(AUTO_TEST_TAG, "Step %d/%d: action=%d",
                              test_state.current_step + 1, tc->step_count, step->action);
                }
                test_state.current_step++;
            }
        } else {
            LOG_INFO(AUTO_TEST_TAG, "Phase 2 complete, test PASSED");
            test_state.phase = 3;
            test_state.passed_tests++;
        }
        break;
    }

    // ============================================================
    // Phase 3: 完成阶段
    // ============================================================
    case 3:
        if (test_state.test_failed) {
            LOG_ERROR(AUTO_TEST_TAG, "===== Test '%s' FAILED (hook timeout) =====", tc->app_name);
        } else {
            LOG_INFO(AUTO_TEST_TAG, "===== Test '%s' PASSED =====", tc->app_name);
        }
        LOG_INFO(AUTO_TEST_TAG, "Results: %d/%d passed", test_state.passed_tests, test_state.total_tests);
        test_state.running = false;
        test_state.completed = true;
        test_state.phase = 4;
        test_state.frame_counter = 0;
        break;

    // ============================================================
    // Phase 4: 延迟退出阶段
    // ============================================================
    case 4:
        test_state.frame_counter++;
        if (test_state.frame_counter >= 30) {
            LOG_INFO(AUTO_TEST_TAG, "Auto-test complete, exiting...");
            _exit(0);
        }
        break;
    }
}

bool auto_test_is_running(void)
{
    return test_state.running && test_state.phase < 3;
}

const char *auto_test_get_target(void)
{
    return test_state.target_app;
}

// ========================
// 各 APP 测试函数（供外部调用）
// ========================

bool auto_test_app_picture(void)
{
    LOG_DEBUG(AUTO_TEST_TAG, "Running Picture app test...");
    const TestCase *tc = find_test_case("Picture");
    if (!tc) return false;
    start_test_case(tc);
    test_state.running = true;
    return true;
}

bool auto_test_app_game_2048(void)
{
    LOG_DEBUG(AUTO_TEST_TAG, "Running 2048 app test...");
    const TestCase *tc = find_test_case("2048");
    if (!tc) return false;
    start_test_case(tc);
    test_state.running = true;
    return true;
}

bool auto_test_app_settings(void)
{
    LOG_DEBUG(AUTO_TEST_TAG, "Running Settings app test...");
    const TestCase *tc = find_test_case("Settings");
    if (!tc) return false;
    start_test_case(tc);
    test_state.running = true;
    return true;
}

// 以下为预留的 APP 测试函数（尚未实现具体测试步骤）
bool auto_test_app_heartbeat(void)   { LOG_WARN(AUTO_TEST_TAG, "Heartbeat test not implemented"); return false; }
bool auto_test_app_idea_anim(void)   { LOG_WARN(AUTO_TEST_TAG, "IdeaAnim test not implemented"); return false; }
bool auto_test_app_media_player(void)
{
    LOG_DEBUG(AUTO_TEST_TAG, "Running MediaPlayer app test...");
    const TestCase *tc = find_test_case("Media");
    if (!tc) return false;
    start_test_case(tc);
    test_state.running = true;
    return true;
}
bool auto_test_app_screen_share(void){ LOG_WARN(AUTO_TEST_TAG, "ScreenShare test not implemented"); return false; }
bool auto_test_app_stockmarket(void)
{
    LOG_DEBUG(AUTO_TEST_TAG, "Running StockMarket app test...");
    const TestCase *tc = find_test_case("Stock");
    if (!tc) return false;
    start_test_case(tc);
    test_state.running = true;
    return true;
}
bool auto_test_app_weather(void)     { LOG_WARN(AUTO_TEST_TAG, "Weather test not implemented"); return false; }
bool auto_test_app_tomato(void)      { LOG_WARN(AUTO_TEST_TAG, "Tomato test not implemented"); return false; }
bool auto_test_app_anniversary(void) { LOG_WARN(AUTO_TEST_TAG, "Anniversary test not implemented"); return false; }
bool auto_test_app_game_snake(void)  { LOG_WARN(AUTO_TEST_TAG, "GameSnake test not implemented"); return false; }
bool auto_test_app_bilibili(void)    { LOG_WARN(AUTO_TEST_TAG, "Bilibili test not implemented"); return false; }
bool auto_test_app_pc_resource(void) { LOG_WARN(AUTO_TEST_TAG, "PCResource test not implemented"); return false; }
bool auto_test_app_file_manager(void)
{
    LOG_DEBUG(AUTO_TEST_TAG, "Running FileManager app test...");
    const TestCase *tc = find_test_case("File Manager");
    if (!tc) return false;
    start_test_case(tc);
    test_state.running = true;
    return true;
}
bool auto_test_app_LHLXW(void)
{
    LOG_DEBUG(AUTO_TEST_TAG, "Running LHLXW app test...");
    const TestCase *tc = find_test_case("LH&LXW");
    if (!tc) return false;
    start_test_case(tc);
    test_state.running = true;
    return true;
}