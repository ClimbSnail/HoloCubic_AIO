/*********************
        INCLUDES
 *********************/
#include "lv_cubic_gui.h"
#include "images.h"
#include "lv_port_indev.h"
#include <string.h>

// 必须定义为全局或者静态
lv_obj_t *app_scr;
lv_obj_t *pre_app_image;
lv_obj_t *now_app_image;
lv_obj_t *wc_scr[4];
lv_obj_t *image_scr;

lv_obj_t *weather_image;
lv_obj_t *cityname_label;
lv_obj_t *temperature_label;

lv_obj_t *time_image;
lv_obj_t *date_label;
lv_obj_t *time_label;

lv_obj_t *cpu_temp_label;
lv_obj_t *cpu_used_label;
lv_obj_t *mem_used_label;
lv_obj_t *net_upload_label;
lv_obj_t *net_download_label;

lv_obj_t *local_ip_label;
lv_obj_t *ap_ip_label;
lv_obj_t *domain_label;
lv_obj_t *title_label;

lv_obj_t *photo_image;

lv_font_declare(lv_font_montserrat_20);
lv_font_declare(lv_font_montserrat_24);
lv_font_declare(lv_font_montserrat_40);

static int pre_app_scr_index = -1; // 标记上一回更新的app页面下标

// APP应用图标
void *app_image_map[APP_NUM] = {&app_weather, &app_picture, &app_movie,
                                &app_screen, &app_server};

// 天气图标路径的映射关系
void *image_map[] = {&Sunny, &Clear, "S:/weather/2.bin", "S:/weather/3.bin",
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

// static lv_group_t *g;
static lv_style_t default_style;
static lv_style_t label_style1;
static lv_style_t label_style2;
static lv_style_t label_style3;
static lv_style_t label_style4;
static lv_style_t label_style5;

void display_init(void)
{
    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_PRESSED, LV_COLOR_GRAY);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED | LV_STATE_PRESSED, lv_color_hex(0xf88));

    // APP图标页
    app_scr = lv_obj_create(NULL, NULL);
    lv_obj_add_style(app_scr, LV_BTN_PART_MAIN, &default_style);

    // photo
    image_scr = lv_obj_create(NULL, NULL);

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

    // 本地的ip地址
    wc_scr[3] = lv_obj_create(NULL, NULL);
    lv_obj_add_style(wc_scr[3], LV_BTN_PART_MAIN, &default_style);
    lv_style_init(&label_style5);
    lv_style_set_text_opa(&label_style5, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&label_style5, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&label_style5, LV_STATE_DEFAULT, &lv_font_montserrat_24);
}

void display_photo_init()
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == image_scr)
        return;
    lv_obj_clean(act_obj); // 清空此前页面
    photo_image = lv_img_create(image_scr, NULL);
}

void display_photo(const char *file_name, lv_scr_load_anim_t anim_type)
{
    display_photo_init();
    char buf[25] = {0};
    sprintf(buf, "S:/image/%s.bin", file_name);
    lv_img_set_src(photo_image, buf);
    lv_obj_align(photo_image, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_scr_load_anim(image_scr, anim_type, 0, 0, false);
}

void display_weather_init()
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == wc_scr[0])
        return;
    lv_obj_clean(act_obj); // 清空此前页面
    weather_image = lv_img_create(wc_scr[0], NULL);
    cityname_label = lv_label_create(wc_scr[0], NULL);
    temperature_label = lv_label_create(wc_scr[0], NULL);
}

void display_weather(const char *cityname, const char *temperature, int weathercode, lv_scr_load_anim_t anim_type)
{
    display_weather_init();
    void *path = image_map[map_index[weathercode]];
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
    lv_obj_clean(act_obj); // 清空此前页面
    time_image = lv_img_create(wc_scr[1], NULL);
    date_label = lv_label_create(wc_scr[1], NULL);
    time_label = lv_label_create(wc_scr[1], NULL);
}

void display_time(const char *date, const char *time, lv_scr_load_anim_t anim_type)
{
    display_time_init();

    lv_img_set_src(time_image, &huoj);
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

void display_setting_init()
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == wc_scr[3])
        return;
    lv_obj_clean(act_obj); // 清空此前页面
    local_ip_label = lv_label_create(wc_scr[3], NULL);
    ap_ip_label = lv_label_create(wc_scr[3], NULL);
    domain_label = lv_label_create(wc_scr[3], NULL);
    title_label = lv_label_create(wc_scr[3], NULL);
}

void display_setting(const char *info, const char *ap_ip, const char *domain, const char *title, lv_scr_load_anim_t anim_type)
{
    display_setting_init();

    lv_obj_add_style(local_ip_label, LV_LABEL_PART_MAIN, &label_style5);
    lv_label_set_text(local_ip_label, info);
    lv_obj_align(local_ip_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 5, -50);

    lv_obj_add_style(ap_ip_label, LV_LABEL_PART_MAIN, &label_style5);
    lv_label_set_text(ap_ip_label, ap_ip);
    lv_obj_align(ap_ip_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 5, -80);

    lv_obj_add_style(domain_label, LV_LABEL_PART_MAIN, &label_style5);
    lv_label_set_text(domain_label, domain);
    lv_obj_align(domain_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 5, -120);

    lv_obj_add_style(title_label, LV_LABEL_PART_MAIN, &label_style5);
    lv_label_set_text(title_label, title);
    lv_obj_align(title_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 5, -200);

    lv_scr_load_anim(wc_scr[3], anim_type, 300, 300, false);
}

void display_app_scr_init(int index)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == app_scr)
        return;
    lv_obj_clean(act_obj); // 清空此前页面
    pre_app_image = lv_img_create(app_scr, NULL);
    lv_img_set_src(pre_app_image, app_image_map[index]);
    lv_obj_align(pre_app_image, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_scr_load_anim(app_scr, LV_SCR_LOAD_ANIM_NONE, 300, 300, false);
}

void display_app_scr(int index, lv_scr_load_anim_t anim_type, bool force)
{
    // force为是否强制刷新页面 true为强制刷新
    if (force == true)
    {
        display_app_scr_init(index);
        pre_app_scr_index = index;
        return;
    }

    if (pre_app_scr_index == index)
        return;
    int now_start_x;
    int now_end_x;
    int old_start_x;
    int old_end_x;

    if (LV_SCR_LOAD_ANIM_MOVE_LEFT == anim_type)
    {
        now_start_x = -128;
        now_end_x = 65;
        old_start_x = 65;
        old_end_x = 240;
    }
    else
    {
        now_start_x = 240;
        now_end_x = 65;
        old_start_x = 65;
        old_end_x = -128;
    }

    now_app_image = lv_img_create(app_scr, NULL);
    lv_img_set_src(now_app_image, app_image_map[index]);
    lv_obj_align(now_app_image, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_anim_path_t path;
    lv_anim_path_init(&path);
    // lv_anim_path_linear lv_anim_path_bounce
    // lv_anim_path_overshoot lv_anim_path_ease_out
    // lv_anim_path_step
    lv_anim_path_set_cb(&path, lv_anim_path_ease_out);
    /*Optional for custom functions*/
    // lv_anim_path_set_user_data(&path, &foo);

    lv_anim_t now_app;
    lv_anim_init(&now_app);
    lv_anim_set_exec_cb(&now_app, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_var(&now_app, now_app_image);
    lv_anim_set_values(&now_app, now_start_x, now_end_x);
    uint32_t duration = lv_anim_speed_to_time(200, now_start_x, now_end_x); // 计算时间
    lv_anim_set_time(&now_app, duration);
    lv_anim_set_path(&now_app, &path); // Default is linear

    lv_anim_t pre_app;
    lv_anim_init(&pre_app);
    lv_anim_set_exec_cb(&pre_app, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_var(&pre_app, pre_app_image);
    lv_anim_set_values(&pre_app, old_start_x, old_end_x);
    duration = lv_anim_speed_to_time(200, old_start_x, old_end_x); // 计算时间
    lv_anim_set_time(&pre_app, duration);
    lv_anim_set_path(&pre_app, &path); // Default is linear

    lv_anim_start(&now_app);
    lv_anim_start(&pre_app);
    ANIEND

    lv_obj_del(pre_app_image); // 删除原先的图像
    pre_app_image = now_app_image;
    pre_app_scr_index = index;
}