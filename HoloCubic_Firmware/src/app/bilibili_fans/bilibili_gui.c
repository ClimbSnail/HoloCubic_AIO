#include "bilibili_gui.h"

#include "lv_port_indev.h"
#include "lvgl.h"

lv_obj_t *bilibili_gui = NULL;
lv_obj_t *follower_label = NULL;

static lv_style_t default_style;
static lv_style_t label_style;

void bilibili_gui_init(void)
{ 
    
}

/*
 * 其他函数请根据需要添加
 */

void display_bilibili(const char *file_name, lv_scr_load_anim_t anim_type)
{
}

void bilibili_gui_del(void)
{
    if (NULL != bilibili_gui)
    {
        lv_obj_clean(bilibili_gui);
        bilibili_gui = NULL;
    }
}