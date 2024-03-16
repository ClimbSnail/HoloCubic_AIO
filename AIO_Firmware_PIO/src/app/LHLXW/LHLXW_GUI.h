#ifndef LV_LHLXW_GUI_H__
#define LV_LHLXW_GUI_H__
#include "lvgl.h"

struct LHLXW_RUN{
    uint8_t option_num;//当前选项序号0-4
    lv_obj_t *LV_LHLXW_GUI_OBJ;//APP UI界面
    lv_obj_t *LV_BACKUP_OBJ;//备份系统界面
    lv_obj_t* option_fun[5];//功能选项
    lv_obj_t* option_label;//选项标签    
};

void LHLXW_GUI_Init(void);
void SWITCH_OPTION(bool _flg_,uint8_t mode);
void LHLXW_GUI_DeInit(void);

#endif

