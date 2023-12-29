#ifndef APP_GAME_2048_GUI_H
#define APP_GAME_2048_GUI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"

// 定义贪吃蛇的移动方向
typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); //等待动画完成

    void game_snake_gui_init(void);
    void generate_food();
    void update_driection(Direction dir);
    void display_snake(int gameStatus, lv_scr_load_anim_t anim_type);
    void game_snake_gui_del(void);

#ifdef __cplusplus
} /* extern "C" */
#endif


#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_game_snake;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif