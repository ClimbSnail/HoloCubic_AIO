/*********************
        INCLUDES
 *********************/
#include "lv_cubic_gui.h"
#include "images.h"
#include "lv_port_indev.h"
#include <string.h>

// 必须定义为全局或者静态
lv_obj_t *scr[4];
lv_obj_t *image;

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
// static lv_obj_t *tv;
// static lv_obj_t *t1;
// static lv_obj_t *t2;
// static lv_obj_t *t3;
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
  lv_obj_add_style(lv_scr_act(), LV_BTN_PART_MAIN, &default_style);

  image = lv_obj_create(NULL, NULL);
  photo_image = lv_img_create(image, NULL);

  // scr[0] = lv_scr_act(); //在第一页创建
  // 天气页初始化
  scr[0] = lv_obj_create(NULL, NULL);
  lv_obj_add_style(scr[0], LV_BTN_PART_MAIN, &default_style);
  lv_style_init(&label_style1);
  lv_style_set_text_opa(&label_style1, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_text_color(&label_style1, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_text_font(&label_style1, LV_STATE_DEFAULT, &lv_font_montserrat_24);
  lv_style_init(&label_style2);
  lv_style_set_text_opa(&label_style2, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_text_color(&label_style2, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_text_font(&label_style2, LV_STATE_DEFAULT, &lv_font_montserrat_40);
  weather_image = lv_img_create(scr[0], NULL);
  cityname_label = lv_label_create(scr[0], NULL);
  temperature_label = lv_label_create(scr[0], NULL);
  
  // 日期时间页初始化
  scr[1] = lv_obj_create(NULL, NULL);
  lv_obj_add_style(scr[1], LV_BTN_PART_MAIN, &default_style);
  lv_style_init(&label_style3);
  lv_style_set_text_opa(&label_style3, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_text_color(&label_style3, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_text_font(&label_style3, LV_STATE_DEFAULT, &lv_font_montserrat_40);
  time_image = lv_img_create(scr[1], NULL);
  date_label = lv_label_create(scr[1], NULL);
  time_label = lv_label_create(scr[1], NULL);

  // 硬件信息页初始化
  scr[2] = lv_obj_create(NULL, NULL);
  lv_obj_add_style(scr[2], LV_BTN_PART_MAIN, &default_style);
  lv_style_init(&label_style4);
  lv_style_set_text_opa(&label_style4, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_text_color(&label_style4, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_text_font(&label_style4, LV_STATE_DEFAULT, &lv_font_montserrat_20);
  cpu_temp_label = lv_label_create(scr[2], NULL);
  cpu_used_label = lv_label_create(scr[2], NULL);
  mem_used_label = lv_label_create(scr[2], NULL);
  net_upload_label = lv_label_create(scr[2], NULL);
  net_download_label = lv_label_create(scr[2], NULL);

  // 本地的ip地址
  scr[3] = lv_obj_create(NULL, NULL);
  lv_obj_add_style(scr[3], LV_BTN_PART_MAIN, &default_style);
  lv_style_init(&label_style5);
  lv_style_set_text_opa(&label_style5, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_text_color(&label_style5, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_text_font(&label_style5, LV_STATE_DEFAULT, &lv_font_montserrat_24);
  local_ip_label = lv_label_create(scr[3], NULL);
  ap_ip_label = lv_label_create(scr[3], NULL);
  domain_label = lv_label_create(scr[3], NULL);
  title_label = lv_label_create(scr[3], NULL);
}

void display_photo(const char *file_name)
{
  char buf[25] = {0};
  sprintf(buf, "S:/image/%s.bin", file_name);
  lv_img_set_src(photo_image, buf);
  lv_obj_align(photo_image, NULL, LV_ALIGN_CENTER, 0, 0);
}

void display_weather(const char *cityname, const char *temperature, int weathercode)
{
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
}

void display_time(const char *date, const char *time)
{
  lv_img_set_src(time_image, &huoj);
  lv_obj_align(time_image, NULL, LV_ALIGN_OUT_TOP_MID, 0, 110);

  lv_obj_add_style(date_label, LV_LABEL_PART_MAIN, &label_style3);
  lv_label_set_text(date_label, date);
  lv_obj_align(date_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 15, -95);

  lv_obj_add_style(time_label, LV_LABEL_PART_MAIN, &label_style3);
  lv_label_set_text(time_label, time);
  lv_obj_align(time_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 65, -50);
}

void display_hardware(const char *info)
{
  lv_obj_add_style(cpu_temp_label, LV_LABEL_PART_MAIN, &label_style4);
  lv_label_set_text_fmt(cpu_temp_label, "CPU Temp: %d °C", 0);
  //lv_obj_set_pos(cpu_temp_label, 2, 30);
  lv_obj_align(cpu_temp_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 2, 30);

  lv_obj_add_style(cpu_used_label, LV_LABEL_PART_MAIN, &label_style4);
  lv_label_set_text_fmt(cpu_used_label, "CPU Used: %d北京\%", 0);
  //lv_obj_set_pos(cpu_used_label, 2, 60);
  lv_obj_align(cpu_used_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 2, 60);

  lv_obj_add_style(mem_used_label, LV_LABEL_PART_MAIN, &label_style4);
  lv_label_set_text_fmt(mem_used_label, "Mem Used: %dMB", 0);
  //lv_obj_set_pos(mem_used_label, 2, 90);
  lv_obj_align(mem_used_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 2, 90);

  lv_obj_add_style(net_upload_label, LV_LABEL_PART_MAIN, &label_style4);
  lv_label_set_text_fmt(net_upload_label, "Net Upload: %dKB/s", 0);
  //lv_obj_set_pos(net_upload_label, 2, 120);
  lv_obj_align(net_upload_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 2, 120);

  lv_obj_add_style(net_download_label, LV_LABEL_PART_MAIN, &label_style4);
  lv_label_set_text_fmt(net_download_label, "Net Download: %dKB/s", 0);
  //lv_obj_set_pos(net_download_label, 2, 150);
  lv_obj_align(net_download_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 2, 150);
}

void display_setting(const char *info, const char *ap_ip, const char *domain, const char *title)
{
  lv_obj_add_style(local_ip_label, LV_LABEL_PART_MAIN, &label_style5);
  lv_label_set_text(local_ip_label, info);
  lv_obj_align(local_ip_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 5, -50);
  //lv_scr_load_anim(local_ip_label, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 300, false);

  lv_obj_add_style(ap_ip_label, LV_LABEL_PART_MAIN, &label_style5);
  lv_label_set_text(ap_ip_label, ap_ip);
  lv_obj_align(ap_ip_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 5, -80);
  //lv_scr_load_anim(local_ip_label, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 300, false);

  lv_obj_add_style(domain_label, LV_LABEL_PART_MAIN, &label_style5);
  lv_label_set_text(domain_label, domain);
  lv_obj_align(domain_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 5, -120);

  lv_obj_add_style(title_label, LV_LABEL_PART_MAIN, &label_style5);
  lv_label_set_text(title_label, title);
  lv_obj_align(title_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 5, -200);
}