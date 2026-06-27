#ifndef ESP_LOG_STUB_H
#define ESP_LOG_STUB_H

#include <stdio.h>

#define ESP_LOGE(tag, format, ...) fprintf(stderr, "E (%s): " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGW(tag, format, ...) fprintf(stderr, "W (%s): " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGI(tag, format, ...) fprintf(stdout, "I (%s): " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGD(tag, format, ...) fprintf(stdout, "D (%s): " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGV(tag, format, ...) fprintf(stdout, "V (%s): " format "\n", tag, ##__VA_ARGS__)

#endif