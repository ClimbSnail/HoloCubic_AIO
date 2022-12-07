#ifndef APP_WEATHER_GUI_H
#define APP_WEATHER_GUI_H

#define MAX_EXTENSION_NUM 5 // 组件最大扩展数

// 遥感器数据，带一位小数的数据均为扩大10倍后的整数部分
struct PC_Resource
{
    int cpu_usage; // CPU利用率(%)
    int cpu_temp;  // CPU温度(℃)，扩大10倍
    int cpu_freq;  // CPU主频(MHz)
    int cpu_power; // CPU功耗(W)，扩大10倍

    int gpu_usage; // GPU利用率(%)
    int gpu_temp;  // GPU温度(℃)，扩大10倍
    int gpu_power; // GPU功耗(W)，扩大10倍

    int ram_usage; // 内存RAM使用率(%)
    int ram_use;   // 内存RAM使用量(MB)

    int net_upload_speed;   // 网络上行速率(KB/s)，扩大10倍
    int net_download_speed; // 网络下行速率(KB/s)，扩大10倍
};

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_pc_resource;

    // 一个组件的传感器集合，适用于CPU，GPU，RAM，SSD, HDD，NET
    // 部分组件可能用不到某个属性，如，RAM用不到温度
    typedef struct _sensor_module
    {
        const char *name;            // 组件名称
        lv_obj_t *module_label;      // 组件标签
        lv_obj_t *usage_rate_arc;    // 组件使用率，弧形工具
        lv_obj_t *usage_rate_label;  // 组件使用率，数值显示标签
        lv_obj_t *buttom_label;      // 组件底部标签，数值显示标签
        lv_obj_t *extension_label_1; // 组件扩展属性1，如CPU主频，GPU显存使用，SSD读速率，NET上行速率等
        lv_obj_t *extension_label_2; // 组件扩展属性2，如SSD写速率，NET下行速率等
        lv_obj_t *extension_label_3; // 组件扩展属性3
        lv_obj_t *extension_label_4; // 组件扩展属性4
        lv_obj_t *extension_label_5; // 组件扩展属性5

        void (*extension_update[MAX_EXTENSION_NUM])(struct PC_Resource); // 扩展属性更新函数指针
    } sensor_module;

    void display_pc_resource_gui_init(void);
    void display_pc_resource_init(void);
    void display_pc_resource(struct PC_Resource sensorInfo);
    void pc_resource_gui_release(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
