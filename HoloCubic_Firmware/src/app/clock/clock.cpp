#include "clock.h"
#include "../sys/app_contorller.h"
#include "clock_gui.h"
// #include "clock_gui.c"

void clock_init()
{
    Serial.println("hello world !");
    Serial.println("[clock] 进入");
    clock_gui_init();
}

void clock_start(AppController *sys, const Imu_Action *info)
{
    if (RETURN == info->active)
    {
        sys->app_exit();
        return;
    }
}

void clock_stop()
{
    Serial.println("[clock] 退出");
}

void clock_notice(APP_EVENT event) {}

APP_OBJ clock_app = {
    "clock",
    &clock_ico,
    clock_init,
    clock_start,
    clock_stop,
    clock_notice};