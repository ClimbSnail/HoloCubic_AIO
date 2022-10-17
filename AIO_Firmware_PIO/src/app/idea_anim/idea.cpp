#include "idea.h"
#include "sys/app_controller.h"
#include "network.h"
#include "common.h"
#include <stdint.h>
#include "ui_animation.h"

#define IDEA_APP_NAME "Idea"

// 由于tft屏幕刷新太慢，现在是先开辟一块屏幕分辨率大小的空间作为一张需要显示的图像，
// 所有的绘图操作在虚拟的空间上，绘制图像，最后调用tft的图像显示功能显示图像

#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 240

uint8_t *screen_buf = NULL;
int choose = 0;

/*这两个函数都只是对图像进行操作，不是对屏幕*/
static void screen_clear(uint16_t color);                            //清屏函数声明
static void screen_draw_pixel(int32_t x, int32_t y, uint16_t color); //描点函数声明
/**********************************/

static void gfx_draw_pixel(int x, int y, unsigned int rgb) //指定GUI库的描点函数
{
    screen_draw_pixel(x, y, rgb);
}

struct EXTERNAL_GFX_OP
{
    void (*draw_pixel)(int x, int y, unsigned int rgb);
    void (*fill_rect)(int x0, int y0, int x1, int y1, unsigned int rgb);
} my_gfx_op;

void screen_clear(uint16_t color)
{
    int32_t i = 0;
    int32_t j = 0;

    for (i = 0; i < SCREEN_HEIGHT; ++i)
    {
        for (j = 0; j < SCREEN_WIDTH; ++j)
        {
            screen_buf[i * SCREEN_WIDTH + j] = color;
        }
    }
}

void screen_draw_pixel(int32_t x, int32_t y, uint16_t color) //指定GUI库的描点函数
{
    if ((x >= SCREEN_WIDTH) || (y >= SCREEN_HEIGHT))
        return;
    if ((x < 0) || (y < 0))
        return;
    screen_buf[y * SCREEN_WIDTH + x] = color;
}

static int idea_init(AppController *sys)
{
    screen_buf = (uint8_t *)malloc(SCREEN_HEIGHT * SCREEN_WIDTH); //动态分配一块屏幕分辨率大小的空间
    if (screen_buf == NULL)
        Serial.println("screen_buf: error");
    else
    {
        Serial.println("screen_buf: OK");
    }
    // Link your LCD driver & start UI:
    my_gfx_op.draw_pixel = gfx_draw_pixel;                       //指定GuiLite库的描点函数
    my_gfx_op.fill_rect = NULL;                                  // gfx_fill_rect;
    create_ui(NULL, SCREEN_WIDTH, SCREEN_HEIGHT, 2, &my_gfx_op); // ui初始化
    screen_clear(0x0000);
    return 0;
}

static void idea_process(AppController *sys,
                         const ImuAction *action)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;

    if (RETURN == action->active)
    {
        sys->app_exit();
        return;
    }

    if (TURN_RIGHT == action->active)
    {
        choose = (choose + 1) % 4;
        screen_clear(0x0000);
        delay(500);
    }
    else if (TURN_LEFT == action->active)
    {
        choose = (choose + 4 - 1) % 4;
        screen_clear(0x0000);
        delay(500);
    }

    //清屏，以黑色作为背景
    screen_clear(0x0000);                                          //增加清除旧显存的代码
    ui_update(choose);                                             // ui更新//最终所有的特效调用都在这里面
    tft->pushImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screen_buf); //显示图像
    delay(20);                                                     //改变这个延时函数就能改变特效播放的快慢
}

static void idea_background_task(AppController *sys,
                                 const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int idea_exit_callback(void *param)
{
    if (NULL != screen_buf)
    {
        free(screen_buf);
        screen_buf = NULL;
    }
    return 0;
}

static void idea_message_handle(const char *from, const char *to,
                                APP_MESSAGE_TYPE type, void *message,
                                void *ext_info)
{
}

APP_OBJ idea_app = {IDEA_APP_NAME, &app_idea, "", idea_init,
                    idea_process, idea_background_task, idea_exit_callback,
                    idea_message_handle};