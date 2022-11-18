#define LV_ATTRIBUTE_IMG_WEATHER_VERSION 2

#include "weather_old_gui.h"

#if LV_ATTRIBUTE_IMG_WEATHER_VERSION == 1
#include "weather_old_image_1.h"
#else
#include "weather_old_image_2.h"
#endif

#include "lvgl.h"

static lv_obj_t *wc_scr[3];

static lv_obj_t *weather_image = NULL;
static lv_obj_t *cityname_label = NULL;
static lv_obj_t *temperature_label = NULL;
static lv_obj_t *temperature_symbol = NULL;

static lv_obj_t *time_image = NULL;
static lv_obj_t *date_label = NULL;
static lv_obj_t *time_label = NULL;

static lv_obj_t *cpu_temp_label = NULL;
static lv_obj_t *cpu_used_label = NULL;
static lv_obj_t *mem_used_label = NULL;
static lv_obj_t *net_upload_label = NULL;
static lv_obj_t *net_download_label = NULL;

// static lv_group_t *g;
static lv_style_t default_style;
static lv_style_t label_style1;
static lv_style_t label_style2;
static lv_style_t label_style3;
static lv_style_t label_style4;

LV_FONT_DECLARE(lv_font_montserrat_20);
LV_FONT_DECLARE(lv_font_montserrat_24);
LV_FONT_DECLARE(lv_font_montserrat_40);

// 天气图标路径的映射关系

#if LV_ATTRIBUTE_IMG_WEATHER_VERSION == 1
const void *image_map[] = {&Sunny, &Clear, "S:/weather/Fair_2.bin", "S:/weather/Fair_3.bin",
                           &Cloudy, &PartlyCloudy, &PartlyCloudy,
                           "S:/weather/MostlyCloudy_7.bin", "S:/weather/MostlyCloudy_8.bin",
                           &Overcast, "S:/weather/Shower_10.bin", &Thundershower,
                           "S:/weather/ThundershowerWithHail_12.bin", &LightRain,
                           &ModerateRain, &HeavyRain, "S:/weather/Storm_16.bin",
                           "S:/weather/HeavyStorm_17.bin", "S:/weather/SevereStorm_18.bin",
                           "S:/weather/IceRain_19.bin", &Sleet, &SnowFlurry, &LightSnow,
                           &ModerateSnow, &HeavySnow, "S:/weather/Snowstorm_25.bin",
                           "S:/weather/Dust_26.bin", "S:/weather/Sand_27.bin", "S:/weather/Duststorm_28.bin",
                           "S:/weather/Sandstorm_29.bin", "S:/weather/Foggy_30.bin", "S:/weather/Haze_31.bin",
                           "S:/weather/Windy_32.bin", "S:/weather/Blustery_33.bin", "S:/weather/Hurricane_34.bin",
                           "S:/weather/TropicalStorm_35.bin", "S:/weather/Tornado_36.bin", "S:/weather/Cold_37.bin",
                           "S:/weather/Hot_38.bin", "S:/weather/Unknown_99.bin"};
#else
const void *image_map[] = {&Sunny_100, &Clear_150, "S:/weather/Clear_150.bin", "S:/weather/Clear_150.bin",
                           &Cloudy_101, &PartlyCloudy_103, &PartlyCloudy_153,
                           "S:/weather/PartlyCloudy_153.bin", "S:/weather/PartlyCloudy_153.bin",
                           &Overcast_104, "S:/weather/ShowerRain_300.bin", &Thundershower_302,
                           "S:/weather/ThundershowerWithHail_304.bin", &LightRain_305,
                           &ModerateRain_306, &HeavyRain_307, "S:/weather/Storm_310.bin",
                           "S:/weather/HeavyStorm_311.bin", "S:/weather/SevereStorm_312.bin",
                           "S:/weather/FreezingRain_313.bin", &Sleet_404, &SnowFlurry_407, &LightSnow_400,
                           &ModerateSnow_401, &HeavySnow_402, "S:/weather/Snowstorm_403.bin",
                           "S:/weather/Dust_504.bin", "S:/weather/Sand_503.bin", "S:/weather/Duststorm_507.bin",
                           "S:/weather/Sandstorm_508.bin", "S:/weather/Foggy_501.bin", "S:/weather/Haze_502.bin",
                           "S:/weather/Windy_32.bin", "S:/weather/Blustery_33.bin", "S:/weather/Hurricane_34.bin",
                           "S:/weather/TropicalStorm_35.bin", "S:/weather/Tornado_36.bin", "S:/weather/Cold_901.bin",
                           "S:/weather/Hot_900.bin", "S:/weather/Unknown_999.bin"};
#endif
const int map_index[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                         10, 11, 12, 13, 14, 15, 16, 17,
                         18, 19, 20, 21, 22, 23, 24, 25,
                         26, 27, 28, 29, 30, 31, 32, 33,
                         34, 35, 36, 37, 38, 99};

void weather_old_obj_del(void);

void weather_old_gui_init(void)
{
    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));

    lv_style_init(&label_style1);
    lv_style_set_text_opa(&label_style1, LV_OPA_COVER);
    lv_style_set_text_color(&label_style1, lv_color_white());
    lv_style_set_text_font(&label_style1, &lv_font_montserrat_24);

    lv_style_init(&label_style2);
    lv_style_set_text_opa(&label_style2, LV_OPA_COVER);
    lv_style_set_text_color(&label_style2, lv_color_white());
    lv_style_set_text_font(&label_style2, &lv_font_montserrat_40);

    lv_style_init(&label_style3);
    lv_style_set_text_opa(&label_style3, LV_OPA_COVER);
    lv_style_set_text_color(&label_style3, lv_color_white());
    lv_style_set_text_font(&label_style3, &lv_font_montserrat_40);

    lv_style_init(&label_style4);
    lv_style_set_text_opa(&label_style4, LV_OPA_COVER);
    lv_style_set_text_color(&label_style4, lv_color_white());
    lv_style_set_text_font(&label_style4, &lv_font_montserrat_20);
}

void display_weather_old_init()
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == wc_scr[0])
        return;

    weather_old_obj_del();
    lv_obj_clean(act_obj); // 清空此前页面

    // 天气页初始化
    wc_scr[0] = lv_obj_create(NULL);
    lv_obj_add_style(wc_scr[0], &default_style, LV_STATE_DEFAULT);

    weather_image = lv_img_create(wc_scr[0]);

    cityname_label = lv_label_create(wc_scr[0]);
    lv_obj_add_style(cityname_label, &label_style1, LV_STATE_DEFAULT);

    temperature_label = lv_label_create(wc_scr[0]);
    lv_obj_add_style(temperature_label, &label_style2, LV_STATE_DEFAULT);

    temperature_symbol = lv_label_create(wc_scr[0]);
    lv_obj_add_style(temperature_symbol, &label_style1, LV_STATE_DEFAULT);
}

void display_weather_old(const char *cityname, const char *temperature, int weathercode, lv_scr_load_anim_t anim_type)
{
    display_weather_old_init();
    const void *path = NULL;
    if (weathercode < 39)
    {
        path = image_map[map_index[weathercode]];
    }
    else
    {
        path = image_map[39];
    }

    lv_img_set_src(weather_image, path);
    lv_label_set_text(cityname_label, cityname);
    lv_label_set_text_fmt(temperature_label, "%s", temperature);
    // LV_ALIGN_RIGHT_MID
    lv_label_set_text_fmt(temperature_symbol, "°C");

    lv_obj_align(weather_image, LV_ALIGN_CENTER, 0, -30);
    lv_obj_align(cityname_label, LV_ALIGN_BOTTOM_LEFT, 20, -30);
    lv_obj_align_to(temperature_label, cityname_label, LV_ALIGN_OUT_RIGHT_BOTTOM, 10, 0);
    lv_obj_align_to(temperature_symbol, temperature_label, LV_ALIGN_OUT_RIGHT_BOTTOM, 10, 0);

    if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    {
        lv_scr_load_anim(wc_scr[0], anim_type, 300, 300, false);
    }
    else
    {
        lv_scr_load(wc_scr[0]);
    }
}

void display_time_old_init()
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == wc_scr[1])
        return;

    weather_old_obj_del();
    lv_obj_clean(act_obj); // 清空此前页面

    // 日期时间页初始化
    wc_scr[1] = lv_obj_create(NULL);
    lv_obj_add_style(wc_scr[1], &default_style, LV_STATE_DEFAULT);

    time_image = lv_img_create(wc_scr[1]);
    date_label = lv_label_create(wc_scr[1]);
    lv_obj_add_style(date_label, &label_style3, LV_STATE_DEFAULT);
    time_label = lv_label_create(wc_scr[1]);
    lv_obj_add_style(time_label, &label_style3, LV_STATE_DEFAULT);
}

void display_time_old(const char *date, const char *time, lv_scr_load_anim_t anim_type)
{
    display_time_old_init();

    lv_img_set_src(time_image, &rocket);
    lv_obj_align(time_image, LV_ALIGN_TOP_MID, 0, 30);

    lv_label_set_text(date_label, date);
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 45);

    lv_label_set_text(time_label, time);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 90);

    if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    {
        lv_scr_load_anim(wc_scr[1], anim_type, 300, 300, false);
    }
    else
    {
        lv_scr_load(wc_scr[1]);
    }
}

void display_hardware_old_init()
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == wc_scr[2])
        return;

    weather_old_obj_del();
    lv_obj_clean(act_obj); // 清空此前页面

    // 硬件信息页初始化
    wc_scr[2] = lv_obj_create(NULL);
    lv_obj_add_style(wc_scr[2], &default_style, LV_STATE_DEFAULT);

    cpu_temp_label = lv_label_create(wc_scr[2]);
    lv_obj_add_style(cpu_temp_label, &label_style4, LV_STATE_DEFAULT);
    cpu_used_label = lv_label_create(wc_scr[2]);
    lv_obj_add_style(cpu_used_label, &label_style4, LV_STATE_DEFAULT);
    mem_used_label = lv_label_create(wc_scr[2]);
    lv_obj_add_style(mem_used_label, &label_style4, LV_STATE_DEFAULT);
    net_upload_label = lv_label_create(wc_scr[2]);
    lv_obj_add_style(net_upload_label, &label_style4, LV_STATE_DEFAULT);
    net_download_label = lv_label_create(wc_scr[2]);
    lv_obj_add_style(net_download_label, &label_style4, LV_STATE_DEFAULT);

    // 绘制
    lv_obj_align(cpu_temp_label, LV_ALIGN_BOTTOM_LEFT, 2, 30);
    lv_obj_align(cpu_used_label, LV_ALIGN_BOTTOM_LEFT, 2, 60);
    lv_obj_align(mem_used_label, LV_ALIGN_BOTTOM_LEFT, 2, 90);
    lv_obj_align(net_upload_label, LV_ALIGN_BOTTOM_LEFT, 2, 120);
    lv_obj_align(net_download_label, LV_ALIGN_BOTTOM_LEFT, 2, 150);
}

void display_hardware_old(const char *info, lv_scr_load_anim_t anim_type)
{
    display_hardware_old_init();

    lv_label_set_text_fmt(cpu_temp_label, "CPU Temp: %d °C", 0);
    lv_label_set_text_fmt(cpu_used_label, "CPU Used: %d北京\%", 0);
    lv_label_set_text_fmt(mem_used_label, "Mem Used: %dMB", 0);
    lv_label_set_text_fmt(net_upload_label, "Net Upload: %dKB/s", 0);
    lv_label_set_text_fmt(net_download_label, "Net Download: %dKB/s", 0);

    lv_scr_load_anim(wc_scr[2], anim_type, 300, 300, false);
}

void weather_old_obj_del(void)
{
    if (NULL != wc_scr[0])
    {
        lv_obj_clean(wc_scr[0]);
        weather_image = NULL;
        cityname_label = NULL;
        temperature_label = NULL;
        temperature_symbol = NULL;
    }

    if (NULL != wc_scr[1])
    {
        lv_obj_clean(wc_scr[1]);
        time_image = NULL;
        date_label = NULL;
        time_label = NULL;
    }

    if (NULL != wc_scr[2])
    {
        lv_obj_clean(wc_scr[2]);
        cpu_temp_label = NULL;
        cpu_used_label = NULL;
        mem_used_label = NULL;
        net_upload_label = NULL;
        net_download_label = NULL;
    }
}

void weather_old_gui_del(void)
{
    for (int pos = 0; pos < 3; ++pos)
    {
        if (NULL != wc_scr[pos])
        {
            lv_obj_clean(wc_scr[pos]); // 清空此前页面
            wc_scr[pos] = NULL;

            weather_image = NULL;
            cityname_label = NULL;
            temperature_label = NULL;
            temperature_symbol = NULL;

            time_image = NULL;
            date_label = NULL;
            time_label = NULL;

            cpu_temp_label = NULL;
            cpu_used_label = NULL;
            mem_used_label = NULL;
            net_upload_label = NULL;
            net_download_label = NULL;
        }
    }

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
    // lv_style_reset(&label_style1);
    // lv_style_reset(&label_style2);
    // lv_style_reset(&label_style3);
    // lv_style_reset(&label_style4);
}