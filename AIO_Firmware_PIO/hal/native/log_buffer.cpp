#include "log_buffer.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define LOG_BUF_SIZE 8192

static char g_log_buf[LOG_BUF_SIZE];
static volatile uint32_t g_write_pos = 0;
static uint32_t g_read_pos = 0;
static HANDLE g_log_thread = NULL;
static volatile bool g_log_running = false;
static volatile int g_log_noise_filter = 0;

static DWORD WINAPI log_consumer_thread(LPVOID param)
{
    (void)param;
    char line[512];
    uint32_t line_len = 0;

    while (g_log_running) {
        uint32_t w = g_write_pos;
        uint32_t r = g_read_pos;

        while (r != w && line_len < sizeof(line) - 1) {
            char c = g_log_buf[r % LOG_BUF_SIZE];
            r++;

            if (c == '\n' || line_len >= (uint32_t)(sizeof(line) - 2)) {
                line[line_len] = c;
                line[line_len + 1] = '\0';
                int skip = 0;
                if (g_log_noise_filter) {
                    if (strstr(line, "[LOOP]") || strstr(line, "[APPCTRL]")) {
                        skip = 1;
                    }
                }
                if (!skip) {
                    fwrite(line, 1, line_len + 1, stdout);
                    fflush(stdout);
                }
                line_len = 0;
            } else {
                line[line_len++] = c;
            }
        }
        g_read_pos = r;

        if (r == w) {
            if (line_len > 0) {
                line[line_len] = '\0';
                fwrite(line, 1, line_len, stdout);
                fflush(stdout);
                line_len = 0;
            }
            Sleep(1);
        }
    }

    if (line_len > 0) {
        line[line_len] = '\0';
        fwrite(line, 1, line_len, stdout);
        fflush(stdout);
    }
    return 0;
}

void log_buffer_init(void)
{
    memset(g_log_buf, 0, sizeof(g_log_buf));
    g_write_pos = 0;
    g_read_pos = 0;
    g_log_running = true;
    g_log_thread = CreateThread(NULL, 0, log_consumer_thread, NULL, 0, NULL);
}

void log_buffer_write(const char *data, uint32_t len)
{
    if (!g_log_running || !data || len == 0) return;

    uint32_t w = (uint32_t)InterlockedExchangeAdd((LONG volatile *)&g_write_pos, (LONG)len);
    for (uint32_t i = 0; i < len; i++) {
        g_log_buf[(w + i) % LOG_BUF_SIZE] = data[i];
    }
}

void log_buffer_flush(void)
{
}

void log_buffer_set_noise_filter(int enable)
{
    g_log_noise_filter = enable;
}

void log_buffer_shutdown(void)
{
    g_log_running = false;
    if (g_log_thread) {
        WaitForSingleObject(g_log_thread, 2000);
        CloseHandle(g_log_thread);
        g_log_thread = NULL;
    }
}