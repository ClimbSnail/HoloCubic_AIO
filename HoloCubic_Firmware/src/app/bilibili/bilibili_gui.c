#include "bilibili_gui.h"

#include "driver/lv_port_indev.h"
#include "lvgl.h"

static lv_obj_t *bilibili_gui = NULL;

static lv_obj_t *fans_label = NULL;
static lv_obj_t *follow_label = NULL;
static lv_obj_t *logo_image = NULL;
static lv_obj_t *avatar_image = NULL;

static lv_style_t default_style;
static lv_style_t fans_label_style;
static lv_style_t follow_label_style;

// static lv_img_decoder_dsc_t img_dc_dsc; // 图片解码器

LV_FONT_DECLARE(lv_font_montserrat_40);
LV_IMG_DECLARE(bilibili_logo_ico);
LV_IMG_DECLARE(avatar_ico);

LV_IMG_DECLARE(imgbtn_green);
LV_IMG_DECLARE(imgbtn_blue);
static lv_style_t imgbtn_style;
static lv_obj_t *imgbtn1;

void bilibili_gui_init(void)
{
    if(NULL == default_style.map)
    {

    }
    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_PRESSED, LV_COLOR_GRAY);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED | LV_STATE_PRESSED, lv_color_hex(0xf88));

    lv_style_init(&fans_label_style);
    lv_style_set_text_opa(&fans_label_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&fans_label_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&fans_label_style, LV_STATE_DEFAULT, &lv_font_montserrat_40);

    lv_style_init(&follow_label_style);
    lv_style_set_text_opa(&follow_label_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&follow_label_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&follow_label_style, LV_STATE_DEFAULT, &lv_font_montserrat_40);
}

void display_bilibili_init(void)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == bilibili_gui)
        return;

    bilibili_obj_del();    // 清空对象
    lv_obj_clean(act_obj); // 清空此前页面

    bilibili_gui = lv_obj_create(NULL, NULL);
    fans_label = lv_label_create(bilibili_gui, NULL);
    follow_label = lv_label_create(bilibili_gui, NULL);
    logo_image = lv_img_create(bilibili_gui, NULL);
    avatar_image = lv_img_create(bilibili_gui, NULL);

    // static lv_res_t res;
    // res = lv_img_decoder_open(&img_dc_dsc, &avatar_image, LV_COLOR_WHITE);
    // if (res == LV_RES_OK)
    // {
    //     /*Do something with `dsc->img_data`*/
    //     lv_img_decoder_close(&img_dc_dsc);
    // }
}

/*
 * 其他函数请根据需要添加
 */

void display_bilibili(const char *file_name, lv_scr_load_anim_t anim_type,
                      const char *fans_num, const char *follow_num)
{
    display_bilibili_init();
    lv_obj_add_style(bilibili_gui, LV_BTN_PART_MAIN, &default_style);

    lv_obj_add_style(fans_label, LV_LABEL_PART_MAIN, &fans_label_style);
    lv_label_set_recolor(fans_label, true);
    lv_label_set_text_fmt(fans_label, "%s", fans_num);
    lv_obj_align(fans_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 60, -90);

    lv_obj_add_style(follow_label, LV_LABEL_PART_MAIN, &fans_label_style);
    lv_label_set_recolor(follow_label, true);
    lv_label_set_text_fmt(follow_label, "%s", follow_num);
    lv_obj_align(follow_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 60, -30);

    lv_img_set_src(logo_image, &bilibili_logo_ico);
    lv_obj_align(logo_image, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
    lv_obj_set_size(logo_image, 200, 62);

    lv_img_set_src(avatar_image, "S:/bilibili/avatar.bin");
    // lv_img_set_zoom(avatar_image, LV_IMG_ZOOM_NONE); // LV_IMG_ZOOM_NONE 为256，不放大不缩小
    // lv_img_set_zoom(avatar_image, 106);
    // lv_img_set_auto_size(avatar_image, true); // 开启自动尺寸 也是默认值
    lv_obj_align(avatar_image, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 20, -30);
    // lv_obj_set_size(avatar_image, 100, 100); // 设置图片尺寸

    lv_scr_load(bilibili_gui);
}

void bilibili_obj_del(void)
{
    if (NULL != fans_label)
    {
        lv_obj_clean(fans_label);
        lv_obj_clean(follow_label);
        lv_obj_clean(logo_image);
        lv_obj_clean(avatar_image);
        fans_label = NULL;
        follow_label = NULL;
        logo_image = NULL;
        imgbtn1 = NULL;
    }
}

void bilibili_gui_del(void)
{
    bilibili_obj_del();
    if (NULL != bilibili_gui)
    {
        lv_obj_clean(bilibili_gui);
        bilibili_gui = NULL;
    }

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
    // lv_style_reset(&fans_label_style);
    // lv_style_reset(&follow_label_style);
}