#include "screen_share_gui.h"
#include "lvgl.h"

lv_obj_t *share_main_scr = NULL;

lv_obj_t *title_label;
lv_obj_t *local_ip_label;
lv_obj_t *local_port_label;
lv_obj_t *info_label;

static lv_style_t default_style;
static lv_style_t label_style;

LV_FONT_DECLARE(lv_font_montserrat_24);

void screen_share_gui_init(void)
{
    share_main_scr = NULL;
    title_label = NULL;
    local_ip_label = NULL;
    local_port_label = NULL;
    info_label = NULL;

    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));

    lv_style_init(&label_style);
    lv_style_set_text_opa(&label_style, LV_OPA_COVER);
    lv_style_set_text_color(&label_style, lv_color_white());
    lv_style_set_text_font(&label_style, &lv_font_montserrat_24);
}

void display_share_init(void)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    // if (act_obj == share_main_scr)
    //     return;

    lv_obj_clean(act_obj); // 清空此前页面
    screen_share_gui_del();

    // 本地的ip地址
    share_main_scr = lv_obj_create(NULL);
    lv_obj_add_style(share_main_scr, &default_style, LV_STATE_DEFAULT);

    title_label = lv_label_create(share_main_scr);
    lv_obj_add_style(title_label, &label_style, LV_STATE_DEFAULT);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 20);

    local_ip_label = lv_label_create(share_main_scr);
    lv_obj_add_style(local_ip_label, &label_style, LV_STATE_DEFAULT);
    lv_obj_align(local_ip_label, LV_ALIGN_BOTTOM_LEFT, 5, -130);

    local_port_label = lv_label_create(share_main_scr);
    lv_obj_add_style(local_port_label, &label_style, LV_STATE_DEFAULT);
    lv_obj_align(local_port_label, LV_ALIGN_BOTTOM_LEFT, 5, -90);

    info_label = lv_label_create(share_main_scr);
    lv_obj_add_style(info_label, &label_style, LV_STATE_DEFAULT);
    lv_obj_align(info_label, LV_ALIGN_BOTTOM_LEFT, 5, -50);

    lv_scr_load(share_main_scr);
}

void display_screen_share(const char *title, const char *ip,
                          const char *port, const char *info,
                          lv_scr_load_anim_t anim_type)
{
    display_share_init();

    lv_label_set_text(title_label, title);

    lv_label_set_text(local_ip_label, ip);

    lv_label_set_text(local_port_label, port);

    lv_label_set_text(info_label, info);
}

void screen_share_gui_del(void)
{
    if (NULL != share_main_scr)
    {
        lv_obj_clean(share_main_scr); // 清空此前页面
        share_main_scr = NULL;
        title_label = NULL;
        local_ip_label = NULL;
        local_port_label = NULL;
        info_label = NULL;
    }

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
    // lv_style_reset(&label_style);
}