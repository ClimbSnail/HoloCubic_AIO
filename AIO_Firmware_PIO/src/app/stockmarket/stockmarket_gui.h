#ifndef APP_STOCKMARKET_GUI_H
#define APP_STOCKMARKET_GUI_H


struct StockMarket
{
    float OpenQuo;              // Today's opening price
    float CloseQuo;             // Yesterday's closing price
    float NowQuo;               // Current price
    float MaxQuo;               // Today's highest price
    float MinQuo;               // Today's lowest price
    float ChgValue;             // Change value
    float ChgPercent;           // Change percentage
    unsigned int updownflag;    // Up/Down flag: 1=up, 0=down
    char name[13];              // Stock name
    char code[9];               // Stock code
    float tradvolume;           // Trading volume
    float turnover;             // Trading value
};

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); // Wait for animation to complete

    void stockmarket_gui_init(void);
    void display_stockmarket(struct StockMarket stockInfo, lv_scr_load_anim_t anim_type);
    void stockmarket_obj_del(void);
    void stockmarket_gui_del(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_stockmarket;
    extern const lv_img_dsc_t stockmarket_logo_ico;
    extern const lv_img_dsc_t down;
    extern const lv_img_dsc_t up;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif