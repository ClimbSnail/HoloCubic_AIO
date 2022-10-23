#ifndef APP_HEARTBEAT_GUI_H
#define APP_HEARTBEAT_GUI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
enum S_R_TYPE{
    SEND = 0,
    RECV,
    HEART,
};
#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); //等待动画完成

    void heartbeat_gui_init(void);
    void display_heartbeat(const char *file_name, lv_scr_load_anim_t anim_type);
    void heartbeat_gui_del(void);
    void heartbeat_obj_del(void);
    void display_heartbeat_img(void);
    void heartbeat_set_sr_type(enum S_R_TYPE type);
    void heartbeat_set_send_recv_cnt_label(uint8_t send_num, uint8_t recv_num);

#ifdef __cplusplus
} /* extern "C" */
#endif


#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_heartbeat;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif