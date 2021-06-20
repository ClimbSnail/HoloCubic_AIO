#ifndef LV_CUBIC_GUI_H
#define LV_CUBIC_GUI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#define ANIEND  while(lv_anim_count_running())lv_task_handler();//等待动画完成

#define APP_NUM 5

	void display_init(void);
	void display_photo(const char *file_name, lv_scr_load_anim_t anim_type);
	void display_weather(const char *cityname, const char * temperature, int weathercode, lv_scr_load_anim_t anim_type);
	void display_time(const char *date, const char *time, lv_scr_load_anim_t anim_type);
	void display_hardware(const char *info, lv_scr_load_anim_t anim_type);
	void display_setting(const char *info, const char *ap_ip, const char *domain, const char *title, lv_scr_load_anim_t anim_type);
    void display_app_scr_init(int index);
    void display_app_scr(int index, lv_scr_load_anim_t anim_type, bool force);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
