/*** 
 * @Author       : zhangmingxin
 * @Date         : 2022-03-21 13:34:25
 * @FilePath     : \lv_platformio\src\bsp_mylvgl.h
 * @Email        : 17360690414@189.com
 * @LastEditTime : 2022-03-21 16:52:39
 */
/***
 * @Author       : zhangmingxin
 * @Date         : 2022-03-21 13:34:25
 * @FilePath     : \lv_platformio\src\bsp_mylvgl.h
 * @Email        : 17360690414@189.com
 * @LastEditTime : 2022-03-21 15:15:07
 */
#ifndef __BSP_MYLVGL_H__
#define __BSP_MYLVGL_H__
#include "lvgl.h"
#define ANIM 1
#define EVENT 0
#define GET_START 0
#define FLIX 0
#define GRID 0
#define BMP 0
#define FRAGMENT 0
#if ANIM
#define ANIM_Example1 1
#define ANIM_Example2 0
#define ANIM_Example3 0
#define ANIM_Example4 0
// anim_example
void lv_example_anim_1(void);
void lv_example_anim_2(void);
void lv_example_anim_3(void);
void lv_example_anim_timeline_1(void);
#endif // ANIM
#if EVENT
#define EVENT1 0
#define EVENT2 0
#define EVENT3 0
#define EVENT4 1
//event_example
void lv_example_event_1(void);
void lv_example_event_2(void);
void lv_example_event_3(void);
void lv_example_event_4(void);
#endif // EVENT
#if GET_START
#define GET_START1 0
#define GET_START2 0
#define GET_START3 1
void lv_example_get_started_1(void);
void lv_example_get_started_2(void);
void lv_example_get_started_3(void);
#endif //GET_START
#if FLIX
#define FLIX1 0
#define FLIX2 0
#define FLIX3 0
#define FLIX4 0
#define FLIX5 0
#define FLIX6 1
void lv_example_flex_1(void);
void lv_example_flex_2(void);
void lv_example_flex_3(void);
void lv_example_flex_4(void);
void lv_example_flex_5(void);
void lv_example_flex_6(void);
#endif //FLIX
#if GRID
#define GRID1 0
#define GRID2 0
#define GRID3 0
#define GRID4 0
#define GRID5 0
#define GRID6 1
void lv_example_grid_1(void);
void lv_example_grid_2(void);
void lv_example_grid_3(void);
void lv_example_grid_4(void);
void lv_example_grid_5(void);
void lv_example_grid_6(void);
#endif //GRID
#if BMP
void lv_example_bmp_1(void);
#endif //BMP
#if FRAGMENT
#define FRAGMENT1 1
void lv_example_fragment_1(void);
#endif //FRAGMENT
#endif //__BSP_MYLVGL_H__
