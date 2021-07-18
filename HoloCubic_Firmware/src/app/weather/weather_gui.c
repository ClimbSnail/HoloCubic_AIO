#include "weather_gui.h"
#include "weather_image.h"

#include "lv_port_indev.h"
#include "lvgl.h"

lv_obj_t *wc_scr[4];

lv_obj_t *weather_image = NULL;
lv_obj_t *cityname_label = NULL;
lv_obj_t *temperature_label = NULL;

lv_obj_t *time_image = NULL;
lv_obj_t *date_label = NULL;
lv_obj_t *time_label = NULL;

lv_obj_t *cpu_temp_label = NULL;
lv_obj_t *cpu_used_label = NULL;
lv_obj_t *mem_used_label = NULL;
lv_obj_t *net_upload_label = NULL;
lv_obj_t *net_download_label = NULL;

// static lv_group_t *g;
static lv_style_t default_style;
static lv_style_t label_style1;
static lv_style_t label_style2;
static lv_style_t label_style3;
static lv_style_t label_style4;

lv_font_declare(lv_font_montserrat_20);
lv_font_declare(lv_font_montserrat_24);
lv_font_declare(lv_font_montserrat_40);

// 天气图标路径的映射关系
const void *image_map[] = {&Sunny, &Clear, "S:/weather/2.bin", "S:/weather/3.bin",
                           &Cloudy, &PartlyCloudy, &Overcast, &ModerateSnow,
                           &Thundershower, "S:/weather/12.bin", &LightRain,
                           &ModerateRain, &HeavyRain, "S:/weather/19.bin",
                           &Sleet, &SnowFlurry, &LightSnow, &HeavySnow,
                           "S:/weather/26.bin", "S:/weather/27.bin", "S:/weather/28.bin",
                           "S:/weather/29.bin", "S:/weather/30.bin", "S:/weather/31.bin",
                           "S:/weather/32.bin", "S:/weather/33.bin", "S:/weather/34.bin",
                           "S:/weather/35.bin", "S:/weather/36.bin", "S:/weather/37.bin",
                           "S:/weather/38.bin", "S:/weather/99.bin"};
int map_index[] = {0, 1, 2, 3, 4, 5, 5, 5, 5, 6,
                   7, 8, 9, 10, 11, 12, 12, 12, 12, 13,
                   14, 15, 16, 7, 17, 17, 18, 19, 20, 21,
                   22, 23, 24, 25, 26, 27, 28, 29, 30};


void weather_obj_del(void);

void weather_gui_init(void)
{
    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_PRESSED, LV_COLOR_GRAY);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED | LV_STATE_PRESSED, lv_color_hex(0xf88));

    // 天气页初始化
    wc_scr[0] = lv_obj_create(NULL, NULL);
    lv_obj_add_style(wc_scr[0], LV_BTN_PART_MAIN, &default_style);
    lv_style_init(&label_style1);
    lv_style_set_text_opa(&label_style1, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&label_style1, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&label_style1, LV_STATE_DEFAULT, &lv_font_montserrat_24);
    lv_style_init(&label_style2);
    lv_style_set_text_opa(&label_style2, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&label_style2, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&label_style2, LV_STATE_DEFAULT, &lv_font_montserrat_40);

    // 日期时间页初始化
    wc_scr[1] = lv_obj_create(NULL, NULL);
    lv_obj_add_style(wc_scr[1], LV_BTN_PART_MAIN, &default_style);
    lv_style_init(&label_style3);
    lv_style_set_text_opa(&label_style3, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&label_style3, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&label_style3, LV_STATE_DEFAULT, &lv_font_montserrat_40);

    // 硬件信息页初始化
    wc_scr[2] = lv_obj_create(NULL, NULL);
    lv_obj_add_style(wc_scr[2], LV_BTN_PART_MAIN, &default_style);
    lv_style_init(&label_style4);
    lv_style_set_text_opa(&label_style4, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&label_style4, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&label_style4, LV_STATE_DEFAULT, &lv_font_montserrat_20);
}

void display_weather_init()
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == wc_scr[0])
        return;
    weather_obj_del();
    lv_obj_clean(act_obj); // 清空此前页面
    weather_image = lv_img_create(wc_scr[0], NULL);
    cityname_label = lv_label_create(wc_scr[0], NULL);
    temperature_label = lv_label_create(wc_scr[0], NULL);
}

void display_weather(const char *cityname, const char *temperature, int weathercode, lv_scr_load_anim_t anim_type)
{
    display_weather_init();
    const void *path = image_map[map_index[weathercode]];
    if (weathercode < 39)
    {
        path = image_map[map_index[weathercode]];
    }
    else
    {
        path = image_map[31];
    }
    lv_img_set_src(weather_image, path);
    lv_obj_align(weather_image, NULL, LV_ALIGN_OUT_TOP_MID, 0, 140);

    lv_obj_add_style(cityname_label, LV_LABEL_PART_MAIN, &label_style1);
    lv_label_set_text(cityname_label, cityname);
    lv_obj_align(cityname_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 20, -65);

    lv_obj_add_style(temperature_label, LV_LABEL_PART_MAIN, &label_style2);
    lv_label_set_text_fmt(temperature_label, "%s°C", temperature);
    lv_obj_align(temperature_label, cityname_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_scr_load_anim(wc_scr[0], anim_type, 300, 300, false);
}

void display_time_init()
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == wc_scr[1])
        return;
    weather_obj_del();
    lv_obj_clean(act_obj); // 清空此前页面
    time_image = lv_img_create(wc_scr[1], NULL);
    date_label = lv_label_create(wc_scr[1], NULL);
    time_label = lv_label_create(wc_scr[1], NULL);
}

void display_time(const char *date, const char *time, lv_scr_load_anim_t anim_type)
{
    display_time_init();

    lv_img_set_src(time_image, &rocket);
    lv_obj_align(time_image, NULL, LV_ALIGN_OUT_TOP_MID, 0, 110);

    lv_obj_add_style(date_label, LV_LABEL_PART_MAIN, &label_style3);
    lv_label_set_text(date_label, date);
    lv_obj_align(date_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 15, -95);

    lv_obj_add_style(time_label, LV_LABEL_PART_MAIN, &label_style3);
    lv_label_set_text(time_label, time);
    lv_obj_align(time_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 65, -50);

    lv_scr_load_anim(wc_scr[1], anim_type, 300, 300, false);
}

void display_hardware_init()
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == wc_scr[2])
        return;
    weather_obj_del();
    lv_obj_clean(act_obj); // 清空此前页面
    cpu_temp_label = lv_label_create(wc_scr[2], NULL);
    cpu_used_label = lv_label_create(wc_scr[2], NULL);
    mem_used_label = lv_label_create(wc_scr[2], NULL);
    net_upload_label = lv_label_create(wc_scr[2], NULL);
    net_download_label = lv_label_create(wc_scr[2], NULL);
}

void display_hardware(const char *info, lv_scr_load_anim_t anim_type)
{
    display_hardware_init();

    lv_obj_add_style(cpu_temp_label, LV_LABEL_PART_MAIN, &label_style4);
    lv_label_set_text_fmt(cpu_temp_label, "CPU Temp: %d °C", 0);
    lv_obj_align(cpu_temp_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 2, 30);

    lv_obj_add_style(cpu_used_label, LV_LABEL_PART_MAIN, &label_style4);
    lv_label_set_text_fmt(cpu_used_label, "CPU Used: %d北京\%", 0);
    lv_obj_align(cpu_used_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 2, 60);

    lv_obj_add_style(mem_used_label, LV_LABEL_PART_MAIN, &label_style4);
    lv_label_set_text_fmt(mem_used_label, "Mem Used: %dMB", 0);
    lv_obj_align(mem_used_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 2, 90);

    lv_obj_add_style(net_upload_label, LV_LABEL_PART_MAIN, &label_style4);
    lv_label_set_text_fmt(net_upload_label, "Net Upload: %dKB/s", 0);
    lv_obj_align(net_upload_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 2, 120);

    lv_obj_add_style(net_download_label, LV_LABEL_PART_MAIN, &label_style4);
    lv_label_set_text_fmt(net_download_label, "Net Download: %dKB/s", 0);
    lv_obj_align(net_download_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 2, 150);

    lv_scr_load_anim(wc_scr[2], anim_type, 300, 300, false);
}

void weather_obj_del(void)
{
    if (NULL != weather_image)
    {
        lv_obj_clean(weather_image);
        lv_obj_clean(cityname_label);
        lv_obj_clean(temperature_label);
        weather_image = NULL;
        cityname_label = NULL;
        temperature_label = NULL;
    }

    if (NULL != time_image)
    {
        lv_obj_clean(time_image);
        lv_obj_clean(date_label);
        lv_obj_clean(time_label);
        time_image = NULL;
        date_label = NULL;
        time_label = NULL;
    }

    if (NULL != cpu_temp_label)
    {
        lv_obj_clean(cpu_temp_label);
        lv_obj_clean(cpu_used_label);
        lv_obj_clean(mem_used_label);
        lv_obj_clean(net_upload_label);
        lv_obj_clean(net_download_label);
        cpu_temp_label = NULL;
        cpu_used_label = NULL;
        mem_used_label = NULL;
        net_upload_label = NULL;
        net_download_label = NULL;
    }
}

void weather_gui_del(void)
{
    weather_obj_del();

    for(int pos=0; pos<4; ++pos)
    {
        if(NULL != wc_scr[0])
        {
            lv_obj_clean(wc_scr[0]); // 清空此前页面
            wc_scr[0] = NULL;
        }
    }
}