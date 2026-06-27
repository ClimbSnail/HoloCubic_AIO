#include "serial_utils.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef NATIVE_SIMULATION
#include "log_buffer.h"
#include <windows.h>
#endif

void serial_printf(const char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);

    unsigned long ts = GetTickCount();
    int ts_len = snprintf(buf, sizeof(buf), "[%06lu] ", ts);
    int len = vsnprintf(buf + ts_len, sizeof(buf) - ts_len, fmt, args);
    va_end(args);

    if (len > 0) {
#ifdef NATIVE_SIMULATION
        log_buffer_write(buf, (uint32_t)(ts_len + len));
#else
        printf("%s", buf);
#endif
    }
}