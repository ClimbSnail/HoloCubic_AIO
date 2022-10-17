#include "heartbeat_gui.h"

#define LV_LVGL_H_INCLUDE_SIMPLE

#include "images/archerS_0000.h"
#include "images/archerS_0001.h"
#include "images/archerS_0002.h"
#include "images/archerS_0003.h"
#include "images/archerS_0004.h"
#include "images/archerS_0005.h"
#include "images/archerS_0006.h"
#include "images/archerS_0007.h"
#include "images/archerS_0008.h"
#include "images/archerR_0000.h"
#include "images/archerR_0001.h"
#include "images/archerR_0002.h"
#include "images/archerR_0003.h"
#include "images/archerR_0004.h"
#include "images/archerR_0005.h"
#include "images/archerR_0006.h"
#include "images/archerR_0007.h"
#include "images/archerR_0008.h"
#include "images/archerR_0009.h"
#include "images/heart_0000.h"
#include "images/heart_0001.h"
#include "images/heart_0002.h"
#include "images/heart_0003.h"
#include "images/heart_0004.h"
#include "images/heart_0005.h"
#include "images/heart_0006.h"
#include "images/heart_0007.h"
#include "images/heart_0008.h"
#include "images/heart_0009.h"
#include "images/heart_0010.h"
#include "images/heart_0011.h"
#include "images/heart_0012.h"
#include "images/heart_0013.h"
#include "images/heart_0014.h"
#include "images/heart_0015.h"
#include "images/heart_0016.h"
#include "images/heart_0017.h"
#include "images/heart_0018.h"
#include "images/heart_0019.h"

#include "driver/lv_port_indev.h"
#include "lvgl.h"

LV_FONT_DECLARE(jb_monob_yahei_22);

#define TEXT_TYPE0_FMT "已发送#00ff45 %02d#次,接收到#ff0000 %02d#次"
#define TEXT_TYPE1_FMT "接收到#ff0000 %02d#次,已发送#00ff45 %02d#次"

#define HEART_IMG_NUM 20
#define RECV_IMG_NUM 10
#define SEND_IMG_NUM 9

const void *archerR_map[] = {&archerR_0000, &archerR_0001, &archerR_0002, &archerR_0003, &archerR_0004,
                             &archerR_0005, &archerR_0006, &archerR_0007, &archerR_0008, &archerR_0009};
const void *archerS_map[] = {&archerS_0000, &archerS_0001, &archerS_0002, &archerS_0003, &archerS_0004,
                             &archerS_0005, &archerS_0006, &archerS_0007, &archerS_0008};
const void *heart_map[] = {
    &heart_0000,
    &heart_0001,
    &heart_0002,
    &heart_0003,
    &heart_0004,
    &heart_0005,
    &heart_0006,
    &heart_0007,
    &heart_0008,
    &heart_0009,
    &heart_0010,
    &heart_0011,
    &heart_0012,
    &heart_0013,
    &heart_0014,
    &heart_0015,
    &heart_0016,
    &heart_0017,
    &heart_0018,
    &heart_0019,
};

static lv_style_t default_style;
static lv_style_t chFont_style;

static enum S_R_TYPE s_r_type = SEND;

static lv_obj_t *heartbeat_gui = NULL;
static lv_obj_t *heartbeatImg = NULL;
static lv_obj_t *txtlabel = NULL;

// lv_timer_t * heartbeat_update_timer = NULL;

void heartbeat_gui_init(void)
{
    // if (NULL == default_style.map)
    // {
    // }

    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));

    lv_style_init(&chFont_style);
    lv_style_set_text_opa(&chFont_style, LV_OPA_COVER);
    lv_style_set_text_color(&chFont_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&chFont_style, &jb_monob_yahei_22);
}

/*
 * 其他函数请根据需要添加
 */

void display_heartbeat(const char *file_name, lv_scr_load_anim_t anim_type)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == heartbeat_gui)
        return;

    heartbeat_gui_del();   // 清空对象
    lv_obj_clean(act_obj); // 清空此前页面

    heartbeat_gui = lv_obj_create(NULL);
    lv_obj_add_style(heartbeat_gui, &default_style, LV_STATE_DEFAULT);

    heartbeatImg = lv_img_create(heartbeat_gui); //创建heart图标

    txtlabel = lv_label_create(heartbeat_gui);
    lv_obj_add_style(txtlabel, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(txtlabel, true);

    lv_label_set_text_fmt(txtlabel, TEXT_TYPE0_FMT, 0, 0);
    s_r_type = SEND;
    display_heartbeat_img();

    // lv_obj_align(heartbeat_gui, LV_ALIGN_CENTER,0,0);
    lv_obj_align(txtlabel, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_align(heartbeatImg, LV_ALIGN_CENTER, 0, 0);
    // if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    // {
    //     lv_scr_load_anim(heartbeat_gui, anim_type, 300, 150, true);
    // }
    // else
    // {
    lv_scr_load(heartbeat_gui);
    // }
}

void display_heartbeat_img(void)
{
    static int _beatIndex = 0;
    if (NULL != heartbeat_gui && lv_scr_act() == heartbeat_gui)
    {
        if (s_r_type == SEND)
        {
            _beatIndex = (_beatIndex + 1) % SEND_IMG_NUM;
            lv_img_set_src(heartbeatImg, archerS_map[_beatIndex]);
        }
        else if (s_r_type == RECV)
        {
            _beatIndex = (_beatIndex + 1) % RECV_IMG_NUM;
            lv_img_set_src(heartbeatImg, archerR_map[_beatIndex]);
        }
        else
        {
            _beatIndex = (_beatIndex + 1) % HEART_IMG_NUM;
            lv_img_set_src(heartbeatImg, heart_map[_beatIndex]);
        }
        lv_obj_align(heartbeatImg, LV_ALIGN_CENTER, 0, 0);
        // lv_img_set_src(heartbeatImg,buf);
    }
}

void heartbeat_set_sr_type(enum S_R_TYPE type)
{
    s_r_type = type;
}

void heartbeat_set_send_recv_cnt_label(uint8_t send_num, uint8_t recv_num)
{
    lv_label_set_text_fmt(txtlabel, TEXT_TYPE0_FMT, send_num, recv_num);
}

void heartbeat_gui_del(void)
{
    if (NULL != heartbeat_gui)
    {
        lv_obj_clean(heartbeat_gui);
        heartbeat_gui = NULL;
        txtlabel = NULL;
        heartbeatImg = NULL;
    }

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
}