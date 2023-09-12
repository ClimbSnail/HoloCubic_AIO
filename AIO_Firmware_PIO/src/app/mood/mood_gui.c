#include "mood_gui.h"

#include "lvgl.h"

lv_obj_t *mood_gui = NULL;

static lv_style_t default_style;
static lv_style_t label_style;

void mood_gui_init(void)
{ 
    lv_style_init(&default_style);
    //设置背景颜色为黑色

}

/*
 * 其他函数请根据需要添加
 */

void display_mood(const char *file_name, lv_scr_load_anim_t anim_type)
{
}

void mood_gui_del(void)
{
    if (NULL != mood_gui)
    {
        lv_obj_clean(mood_gui);
        mood_gui = NULL;
    }
    
    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
}