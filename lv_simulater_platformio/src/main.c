/*
 * @Author       : zhangmingxin
 * @Date         : 2022-03-18 10:14:24
 * @FilePath     : \lv_platformio\src\main.c
 * @Email        : 17360690414@189.com
 * @LastEditTime : 2022-03-21 16:36:48
 */
/**
 ******************************************************************************
 * @file    main.c
 * @author  Ac6
 * @version V1.0
 * @date    01-December-2013
 * @brief   Default main function.
 ******************************************************************************
 */

#include "bsp_mylvgl.h"
#include "lvgl.h"
#include "app_hal.h"


LV_IMG_DECLARE(app_weather);
void test_ui()
{
  static lv_obj_t* default_scr;
  default_scr = lv_scr_act();
  lv_obj_t* img_test_1 = lv_img_create(default_scr);
  lv_obj_align(img_test_1, LV_ALIGN_CENTER, 0, -20);
  lv_img_set_src(img_test_1, &app_weather);

  lv_obj_t* label = lv_label_create(default_scr);
  lv_label_set_text(label, "weather");
  lv_obj_align_to(label,img_test_1,LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
  
}

int main(void)
{
  lv_init();

  hal_setup();
  // lv_example_anim_1();
  // lv_demo_widgets();
  test_ui();
  LV_LOG_USER("test");
  
  hal_loop();
}
