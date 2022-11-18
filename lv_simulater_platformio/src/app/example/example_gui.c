#include "example_gui.h"

#include "lvgl.h"

lv_obj_t *example_gui = NULL;

static lv_style_t default_style;
static lv_style_t label_style;

void example_gui_init(void)
{ 
    lv_style_init(&default_style);
}

/*
 * 其他函数请根据需要添加
 */

void display_example(const char *file_name, lv_scr_load_anim_t anim_type)
{
}

void example_gui_del(void)
{
    if (NULL != example_gui)
    {
        lv_obj_clean(example_gui);
        example_gui = NULL;
    }
    
    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
}