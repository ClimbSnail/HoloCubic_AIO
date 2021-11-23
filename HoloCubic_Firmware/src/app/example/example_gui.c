#include "example_gui.h"

#include "driver/lv_port_indev.h"
#include "lvgl.h"

lv_obj_t *example_gui = NULL;

static lv_style_t default_style;
static lv_style_t label_style;

void example_gui_init(void)
{ 
    
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
}