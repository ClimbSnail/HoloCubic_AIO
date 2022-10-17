#include "file_manager_gui.h"
#include "lvgl.h"

lv_obj_t *file_manager_scr = NULL;

lv_obj_t *title_label;
lv_obj_t *ap_ip_label;
lv_obj_t *port_label;
lv_obj_t *info_label;

static lv_style_t default_style;
static lv_style_t label_style5;

LV_FONT_DECLARE(lv_font_montserrat_24);

void file_maneger_gui_init(void)
{
    file_manager_scr = NULL;
    title_label = NULL;
    ap_ip_label = NULL;
    port_label = NULL;
    info_label = NULL;

    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));

    lv_style_init(&label_style5);
    lv_style_set_text_opa(&label_style5, LV_OPA_COVER);
    lv_style_set_text_color(&label_style5, lv_color_white());
    lv_style_set_text_font(&label_style5, &lv_font_montserrat_24);
}

void display_file_manager_init(void)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == file_manager_scr)
        return;
    
    lv_obj_clean(act_obj); // 清空此前页面

    // 本地的ip地址
    file_manager_scr = lv_obj_create(NULL);
    lv_obj_add_style(file_manager_scr, &default_style, LV_STATE_DEFAULT);

    title_label = lv_label_create(file_manager_scr);
    lv_obj_add_style(title_label, &label_style5, LV_STATE_DEFAULT);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 20);

    ap_ip_label = lv_label_create(file_manager_scr);
    lv_obj_add_style(ap_ip_label, &label_style5, LV_STATE_DEFAULT);
    lv_obj_align(ap_ip_label, LV_ALIGN_BOTTOM_LEFT, 5, -90);

    port_label = lv_label_create(file_manager_scr);
    lv_obj_add_style(port_label, &label_style5, LV_STATE_DEFAULT);
    lv_obj_align(port_label, LV_ALIGN_BOTTOM_LEFT, 5, -50);

    info_label = lv_label_create(file_manager_scr);
    lv_obj_add_style(info_label, &label_style5, LV_STATE_DEFAULT);
    lv_obj_align(info_label, LV_ALIGN_BOTTOM_LEFT, 5, -20);

    lv_scr_load(file_manager_scr);
}

void display_file_manager(const char *title, const char *ap_ip,
                          const char *port, const char *info,
                          lv_scr_load_anim_t anim_type)
{
    display_file_manager_init();

    lv_label_set_text(title_label, title);

    lv_label_set_text(ap_ip_label, ap_ip);

    lv_label_set_text(port_label, port);

    lv_label_set_text(info_label, info);
}

void file_manager_gui_del(void)
{
    if (NULL != file_manager_scr)
    {
        lv_obj_clean(file_manager_scr); // 清空此前页面
        file_manager_scr = NULL;
        port_label = NULL;
        ap_ip_label = NULL;
        info_label = NULL;
        title_label = NULL;
    }

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
    // lv_style_reset(&label_style5);
}