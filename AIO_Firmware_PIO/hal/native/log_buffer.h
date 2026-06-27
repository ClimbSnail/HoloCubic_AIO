#ifndef LOG_BUFFER_H
#define LOG_BUFFER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void log_buffer_init(void);
void log_buffer_write(const char *data, uint32_t len);
void log_buffer_flush(void);
void log_buffer_shutdown(void);

void log_buffer_set_noise_filter(int enable);

#ifdef __cplusplus
}
#endif

#endif