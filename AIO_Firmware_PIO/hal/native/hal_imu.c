#include "hal_native.h"
#include <stdio.h>

// 从 hal_display.c 获取统一事件循环中的键盘动作
extern int hal_native_get_key_action(void);

int hal_imu_get_action(void)
{
    return hal_native_get_key_action();
}

void hal_imu_get_motion(int16_t *ax, int16_t *ay, int16_t *az,
                        int16_t *gx, int16_t *gy, int16_t *gz)
{
    if (ax) *ax = 0;
    if (ay) *ay = 0;
    if (az) *az = 16384;
    if (gx) *gx = 0;
    if (gy) *gy = 0;
    if (gz) *gz = 0;
}