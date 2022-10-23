#include "server_gui.h"
#include "lvgl.h"

static lv_obj_t *main_scr = NULL;

static lv_obj_t *local_ip_label;
static lv_obj_t *ap_ip_label;
static lv_obj_t *domain_label;
static lv_obj_t *title_label;

static lv_style_t default_style;
static lv_style_t label_style;

LV_FONT_DECLARE(lv_font_montserrat_24);

void server_gui_init(void)
{
    main_scr = NULL;
    local_ip_label = NULL;
    ap_ip_label = NULL;
    domain_label = NULL;
    title_label = NULL;

    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));
    
    lv_style_init(&label_style);
    lv_style_set_text_opa(&label_style, LV_OPA_COVER);
    lv_style_set_text_color(&label_style, lv_color_white());
    lv_style_set_text_font(&label_style, &lv_font_montserrat_24);
}

void display_setting_init(void)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == main_scr)
        return;

    if (NULL != main_scr)
    {
        lv_obj_clean(main_scr);
    }

    lv_obj_clean(act_obj); // 清空此前页面

    // 本地的ip地址
    main_scr = lv_obj_create(NULL);
    lv_obj_set_size(main_scr, 240 , 240);
    lv_obj_set_pos(main_scr, 0, 0);
    lv_obj_add_style(main_scr, &default_style, LV_STATE_DEFAULT);

    title_label = lv_label_create(main_scr);
    lv_obj_add_style(title_label, &label_style, LV_STATE_DEFAULT);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 20);

    domain_label = lv_label_create(main_scr);
    lv_obj_add_style(domain_label, &label_style, LV_STATE_DEFAULT);
    lv_obj_align(domain_label, LV_ALIGN_BOTTOM_LEFT, 5, -90);

    ap_ip_label = lv_label_create(main_scr);
    lv_obj_add_style(ap_ip_label, &label_style, LV_STATE_DEFAULT);
    lv_obj_align(ap_ip_label, LV_ALIGN_BOTTOM_LEFT, 5, -50);

    local_ip_label = lv_label_create(main_scr);
    lv_obj_add_style(local_ip_label, &label_style, LV_STATE_DEFAULT);
    lv_obj_align(local_ip_label, LV_ALIGN_BOTTOM_LEFT, 5, -20);

    lv_scr_load(main_scr);
}

void display_setting(const char *title, const char *domain,
                     const char *info, const char *ap_ip,
                     lv_scr_load_anim_t anim_type)
{
    display_setting_init();

    lv_label_set_text(title_label, title);

    lv_label_set_text(domain_label, domain);

    lv_label_set_text(ap_ip_label, ap_ip);

    lv_label_set_text(local_ip_label, info);
}

void setting_gui_del(void)
{
    if (NULL != main_scr)
    {
        lv_obj_clean(main_scr); // 清空此前页面
        main_scr = NULL;
        local_ip_label = NULL;
        ap_ip_label = NULL;
        domain_label = NULL;
        title_label = NULL;
    }

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
    // lv_style_reset(&label_style);
}