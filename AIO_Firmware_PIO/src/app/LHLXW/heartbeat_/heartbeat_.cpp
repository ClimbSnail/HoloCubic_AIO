#include "sys/app_controller.h"
#include "lvgl.h"
#include "stdlib.h"
#include "math.h"


/*
功能：动态心
参考了fzjpxlay up主的代码！
以及他分享的粒子动画网页https://openprocessing.org/sketch/1906399
上述网站可以称之为粒子动画神仙网站，上面有很多惊艳的粒子动画特效
*/


/* 系统变量 */
extern bool isCheckAction;
extern ImuAction *act_info;

static uint8_t *heartbeatBuf = NULL;
static void heartbeatBuf_clear(uint16_t color){
    uint8_t i = 0;
    uint8_t j = 0;
    for (i = 0; i < 240; ++i)
        for (j = 0; j < 240; ++j)
            heartbeatBuf[i * 240 + j] = color;
}
static void heartbeatBuf_draw_pixel(int16_t x, int16_t y, uint16_t color){
    if ((x >= 240) || (y >= 240))
        return;
    if ((x < 0) || (y < 0))
        return;
    heartbeatBuf[y * 240 + x] = color;
}

//h = 0..359, s = 0..100, v = 0..100
//randomColorHSV(330,45+359,80,100,90,100);
static uint16_t randomColorHSV(int hmin,int hmax,int smin,int smax,int vmin,int vmax){
    float h = (((float)rand()/(RAND_MAX))*((float)(hmax-hmin))+(float)hmin);
    h = h>360?h-360:h;
    float s = (((float)rand()/(RAND_MAX))*((float)(smax-smin))+(float)smin);
    float v = (((float)rand()/(RAND_MAX))*((float)(vmax-vmin))+(float)vmin);
    lv_color_t c = lv_color_hsv_to_rgb(h, s, v);
    return (lv_color_to8(c));
}

/*********填充圆用到的工具函数********/
static void heartbeatBuf_draw_fastVLine(int16_t x, int16_t y, int16_t length, uint16_t color){
    int16_t y0 = y;
    do{
        heartbeatBuf_draw_pixel(x, y, color);
        y++;
    } while (y0 + length >= y);
}

static void heartbeatBuf_fillCircle_helper(int16_t x0, int16_t y0, int16_t r, int16_t corner, int32_t delta, uint16_t color){
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    while (x < y){
        if (f >= 0){
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if (corner & 0x1){
            heartbeatBuf_draw_fastVLine(x0 + x, y0 - y, 2 * y + delta, color);
            heartbeatBuf_draw_fastVLine(x0 + y, y0 - x, 2 * x + delta, color);
        }
        if (corner & 0x2){
            heartbeatBuf_draw_fastVLine(x0 - x, y0 - y, 2 * y + delta, color);
            heartbeatBuf_draw_fastVLine(x0 - y, y0 - x, 2 * x + delta, color);
        }
    }
}
static void heartbeatBuf_fill_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color){
    heartbeatBuf_draw_fastVLine(x0, y0 - r, 2 * r, color);
    heartbeatBuf_fillCircle_helper(x0, y0, r, 3, 0, color);
}

#define CIRCLE_TOTAL 81         //粒子总数
#define VISCOSITY 0.95       //粘性系数
#define CIRCLE_SIZE 250         //粒子大小
/* 在此文件中，在类中写函数会报错 */
struct heartCircle {
    float xPos;
    float yPos;
    float xVel;
    float yVel;
    float mass;
    uint16_t color;
    int32_t R;
};

static void circle_move(heartCircle *c){
    c->xPos += c->xVel;
    c->yPos += c->yVel;
}
static void circle_draw(heartCircle *c){
    heartbeatBuf_fill_circle(c->xPos, c->yPos,c->R, c->color);
}


heartCircle *h_circles = NULL; //组成心的圆


static float _absF(float a){
    return a > 0 ? a : -a;
}
static void heartbeatBuf_update(float accXinc, float accYinc){
    for (int i = 0; i < CIRCLE_TOTAL; i++){
        float accX = 0; float accY = 0;
        bool flagUse = true;
        float disXL = (120 - h_circles[i].xPos);
        float disYL = (120 - h_circles[i].yPos);
        float polarR = sqrt(disXL * disXL + disYL * disYL);
        float polarT = atan2(disYL , disXL);

        if (polarR > 80/sqrt(1-_absF(cos(polarT))*sin(polarT))){
            accX += 0.4 * disXL;
            accY += 0.4 * disYL;
            flagUse = false;
        }

        for (int j = 0; j < CIRCLE_TOTAL; j++){
            if (flagUse){
                if (i != j){
                    float x = h_circles[j].xPos - h_circles[i].xPos;
                    float y = h_circles[j].yPos - h_circles[i].yPos;
                    float dis = sqrt(x * x + y * y);
                    if (dis < 1) dis = 1;
    
                    float force = (dis - 120) * h_circles[j].mass / dis;
                    accX += force * x;
                    accY += force * y;
                }
            }
        }
        accX += accXinc;
        accY += accYinc;
        h_circles[i].xVel = h_circles[i].xVel * VISCOSITY + accX * h_circles[i].mass;
        h_circles[i].yVel = h_circles[i].yVel * VISCOSITY + accY * h_circles[i].mass;
        circle_move(&h_circles[i]);
        circle_draw(&h_circles[i]);
    }
}




void heartbeat_init(void){
    heartbeatBuf = (uint8_t *)malloc(240 * 240); //动态分配一块屏幕分辨率大小的空间
    if(heartbeatBuf == NULL){
        Serial.println("1:lack of memory");
        while(1);
    }  
    h_circles = (heartCircle *)calloc(1, sizeof(heartCircle)*CIRCLE_TOTAL);
    if(h_circles == NULL){
        Serial.println("2:lack of memory");
        while(1);
    }  
    tft->fillRect(0, 0, 240, 240, 0);
    for (uint8_t i = 0; i < CIRCLE_TOTAL; i++){
        h_circles[i].xPos = ((float)rand()/RAND_MAX)*240; 
        h_circles[i].yPos = ((float)rand()/RAND_MAX)*240;
        h_circles[i].xVel = 0; 
        h_circles[i].yVel = 0;
        h_circles[i].mass = ((float)rand()/RAND_MAX)*0.03 + 0.005;
        h_circles[i].color = randomColorHSV(330,45+359,80,100,90,100);
        h_circles[i].R = (int32_t)(CIRCLE_SIZE * h_circles[i].mass);
    }
}

void heartbeat_process(lv_obj_t *ym){
    /*实现丝滑切换*/
    lv_obj_t *obj = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(obj,lv_color_hex(0),LV_STATE_DEFAULT);
    lv_scr_load_anim(obj, LV_SCR_LOAD_ANIM_OUT_BOTTOM, 573, 0, false);
    for(uint16_t i=0;i<573;i++){
        lv_timer_handler();//让LVGL更新屏幕，让操作者可以看到已执行动作
        delay(1);//
    }

    float accXinc = 0;
    float accYinc = 0;
    heartbeat_init();
    while(1){
        /* MPU6050数据获取 */
        if (isCheckAction){
            isCheckAction = false;
            act_info = mpu.getAction();
        }
        accXinc = 0;
        accYinc = 0;
        /* MPU6050动作响应 */
        if (RETURN == act_info->active){
            break;
        }else if(TURN_RIGHT == act_info->active){
            accXinc = 80;
            accYinc = 0;
        }else if(TURN_LEFT == act_info->active){
            accXinc = -80;
            accYinc = 0;
        }else if(act_info->active == UP){
            accXinc = 0;
            accYinc = -80;
        }else if(act_info->active == DOWN){
            accXinc = 0;
            accYinc = 80;
        }
        act_info->active = ACTIVE_TYPE::UNKNOWN;
        act_info->isValid = 0;
        heartbeatBuf_clear(0x0000);//清屏，以黑色作为背景 
        heartbeatBuf_update(accXinc,accYinc); // ui更新//最终所有的特效调用都在这里面
        tft->pushImage(0, 0, 240, 240, heartbeatBuf);//显示图像
        delay(2);    
    }
    free(h_circles);
    free(heartbeatBuf);

    lv_scr_load_anim(ym, LV_SCR_LOAD_ANIM_OUT_TOP, 573, 0, false);//调用系统退出函数之前，一定要等待动画结束否则会导致系统重启
    lv_obj_invalidate(lv_scr_act());//哪怕缓存没变，也让lvgl下次更新全部屏幕
    /* 延时999ms，防止同时退出app */
    for(uint16_t i=0;i<898+500;i++){
        lv_timer_handler();//让LVGL更新屏幕，让操作者可以看到已执行动作
        delay(1);//
    }
    lv_obj_clean(obj);
    lv_obj_del(obj);
}


