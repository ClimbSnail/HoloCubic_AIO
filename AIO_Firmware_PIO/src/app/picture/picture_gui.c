#include "picture_gui.h"

#include "lvgl.h"
#include "stdio.h"

lv_obj_t *image_scr = NULL;
lv_obj_t *photo_image = NULL;

static lv_style_t default_style;

void photo_gui_init()
{
    image_scr = lv_obj_create(NULL);

    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));
    // lv_style_set_bg_color(&default_style, lv_palette_main(lv_palette_t p));
    // lv_style_set_bg_color(&default_style, lv_color_black());

    lv_obj_add_style(image_scr, &default_style, LV_STATE_DEFAULT);
}

void display_photo_init()
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == image_scr)
        return;
    lv_obj_clean(act_obj); // 清空此前页面
    photo_image = lv_img_create(image_scr);
}

void display_photo(const char *file_name, lv_scr_load_anim_t anim_type)
{
    display_photo_init();
    char lv_file_name[PIC_FILENAME_MAX_LEN] = {0};
    sprintf(lv_file_name, "S:%s", file_name);
    lv_img_set_src(photo_image, lv_file_name);
    lv_obj_align(photo_image, LV_ALIGN_CENTER, 0, 0);
    lv_scr_load_anim(image_scr, anim_type, 0, 0, false);
}

void photo_gui_del(void)
{
    if (NULL != photo_image)
    {
        lv_obj_clean(photo_image); // 清空此前页面
        photo_image = NULL;
    }

    if (NULL != image_scr)
    {
        lv_obj_clean(image_scr); // 清空此前页面
        image_scr = NULL;
    }

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
}