#include "emoji.h"
#include "stdlib.h"//print
#include "emoji_GUI.h"
#include "../LHLXW.h"

/*
功能：QQ超级表情
操作说明：左/右选择不同表情，下仰播放当前表情，后仰退出当前播放/退出当前功能
其他：
QQ超级表情是一中lottie动画，单片机直接解析lottie动画比较困难（但可以实现）
关于如何获取QQ超级表情见：
【透明小电视&QQ超级表情】 https://www.bilibili.com/video/BV1dC4y1q7R7/?share_source=copy_web&vd_source=68337adbea96c8cef50403a4b2809df6

emoji功能依赖于media_player APP中的视频播放库

SD卡存放说明(emoji相关功能):
./LH&LXW/emoji/videos/videox.mjpeg 存放要播放的视频（大小240x240）(x为0~99)
./LH&LXW/emoji/images/imagex.bin   存放要播放的视频的封面（大小60x60）(x为0~99)
./LH&LXW/emoji/emoji_num.txt        存放要播放的视频数(00~99) 例如7个视频，写07

./LH&LXW/emoji/videos/中的视频数必须等于./LH&LXW/emoji/images/中的封面数
同时必须等于./LH&LXW/emoji/emoji_num.txt中用户输入的视频个数

封面要由lvgl官网img工具工具生成，多功能上位机无法生成

所以将此功能当作视频播放器的时候，添加自定义视频的步骤如下：
1.获取视频，用多功能上位机转为240x240的mjpeg格式（参数推荐用默认的）
2.将此视频名字更改为videoxx.mjpeg
3.从此视频中截取一张图片，长宽比尽量1:1
4.将此图片重命名为imagexx.jpg
5.通过win自带的画图将其尺寸修改为60x60（修改时不用保持原比例）
6.用lvgl官网的img工具将其转换为imagexx.bin
7.将imagexx.bin和videoxx.mjpeg放到SD卡对应文件夹
8.修改emoji_num.txt中的视频总数
9.打开APP,在emoji功能就能看到对应的视频
(如果想实现视频黑色背景，可以用剪辑软件处理；也可以用openCV处理，但只支持背景颜色是纯色；
openCV一次性替换所有QQ超级表情背景颜色：
【QQ超级表情背景颜色替换】 https://www.bilibili.com/video/BV1cb4y1G7Sy/?share_source=copy_web&vd_source=68337adbea96c8cef50403a4b2809df6)
*/

/* 系统变量 */
extern bool isCheckAction;
extern ImuAction *act_info;

#define emoji_play_time 33333//一个表情播放(emoji_play_time)ms自动播放下一个，也可以手动切换

EMOJI_RUN *emj_run = NULL;

static void emoji_init(void){
    emj_run = (EMOJI_RUN*)calloc(1,sizeof(EMOJI_RUN));//如果这里用malloc就会导致视频播放失败！！！
    if(emj_run == NULL){
        Serial.println("1:lack of memory");
        while(1);
    }  
    emj_run->emoji_var = 1;//复位当前选中的表情序号
    emj_run->mpu6050key_var = 0;//进入APP后，mpu6050的状态值，左倾斜为2，右倾为1，其余为0
    emj_run->emoji_mode = true;//app运行模式，true为选择表情，false为表情播放
    File dataFile = SD.open("/LH&LXW/emoji/emoji_num.txt","r");//建立File对象用于从SPIFFS中读取文件
    emj_run->emoji_Maxnum = (dataFile.read() - '0')*10;
    emj_run->emoji_Maxnum += (dataFile.read() - '0');//总共有多少个表情（SPIFFS不会用，所以人为输入个数，即读取SD卡配置文件)
    Serial.print(emj_run->emoji_Maxnum);
    dataFile.close();//读取完毕后，关闭文件
    EMOJI_GUI_Init();
}

/* 关闭播放 */
static void close_player(void){
    delete emj_run->emoji_docoder;
    emj_run->emoji_file.close();
}
/* 开启播放 */
static void start_player(void){
    char *path = (char*)malloc(38);//必须用char*类型，不能用uint8_t*
    sprintf(path,"/LH&LXW/emoji/videos/video%d.mjpeg",emj_run->emoji_var);//图标路径
    emj_run->emoji_file = tf.open(path);
    emj_run->emoji_docoder = new MjpegPlayDocoder(&emj_run->emoji_file, true);
    free(path);  
}
void emoji_process(lv_obj_t *ym)
{
    unsigned long *timCont = (unsigned long*)malloc(4);
    emoji_init();
    act_info->active = ACTIVE_TYPE::UNKNOWN;
    act_info->isValid = 0;
    while(1){
        /* 表情选择时才刷新lvgl */
        if(emj_run->emoji_mode)lv_timer_handler();
        else{
            if(emj_run->emoji_file.available()){
                emj_run->emoji_docoder->video_play_screen();// 播放一帧数据
            }else{
                /* 判断有没有超过3333ms */
                if(millis()-(*timCont) > emoji_play_time){
                    next_emoji();//更新表情选择页面选中的表情
                    emj_run->emoji_var ++;
                    if(emj_run->emoji_var > emj_run->emoji_Maxnum)emj_run->emoji_var = 1;
                    *timCont = millis();
                }
                close_player();
                start_player();
                emj_run->emoji_docoder->video_play_screen();//立即播放一帧数据
            }
        }
        /* MPU6050数据获取 */
        if (isCheckAction){
            isCheckAction = false;
            act_info = mpu.getAction();
        }

        /* MPU6050动作响应 */
        if (RETURN == act_info->active){
            /* 表情选择时，后仰退出app */
            if(emj_run->emoji_mode){
                EMOJI_GUI_DeInit(ym);//退出APP时有LVGL动画，故要等动画结束才能调用系统退出函数，所以UI退出不能放在LHLXW_exit_callback中
                lv_obj_invalidate(lv_scr_act());//哪怕缓存没变，也让lvgl下次更新全部屏幕
                /* 延时999ms，防止同时退出app */
                for(uint16_t i=0;i<898;i++){
                    lv_timer_handler();//让LVGL更新屏幕，让操作者可以看到已执行动作
                    delay(1);//
                }
                // close_player();//此处一定是关闭播放状态的，再调用系统必崩
                free(emj_run);//释放内存
                return;//退出此功能
            }
            /* 表情播放时，后仰退出表情播放 */
            else{
                emj_run->emoji_mode = true;
                close_player();//关闭播放
                lv_obj_invalidate(lv_scr_act());//哪怕缓存没变，也让lvgl下次更新全部屏幕
                /* 延时999ms，防止同时退出emoji功能 */
                for(uint16_t i=0;i<898;i++){
                    lv_timer_handler();//让LVGL更新屏幕，让操作者可以看到已执行动作
                    delay(1);//
                }
            }
        }else if(TURN_RIGHT == act_info->active && emj_run->mpu6050key_var != 1){
            if(emj_run->emoji_mode)
                emj_run->mpu6050key_var = 1;    
            else{
                next_emoji();//更新表情选择页面选中的表情
                emj_run->emoji_var ++;
                if(emj_run->emoji_var > emj_run->emoji_Maxnum)emj_run->emoji_var = 1;
                *timCont = millis();//重新开始计时
                close_player();
                start_player();
            }
            for(uint16_t i=0;i<388;i++){
                if(emj_run->emoji_mode)lv_timer_handler();//
                delay(1);//
            }
        }else if(TURN_LEFT == act_info->active && emj_run->mpu6050key_var != 2){
            if(emj_run->emoji_mode)
                emj_run->mpu6050key_var = 2;    
            else{
                prev_emoji();//更新表情选择页面选中的表情
                emj_run->emoji_var --;
                if(emj_run->emoji_var < 1)emj_run->emoji_var = emj_run->emoji_Maxnum;
                *timCont = millis();//重新开始计时
                close_player();
                start_player();
            }
            for(uint16_t i=0;i<388;i++){
                if(emj_run->emoji_mode)lv_timer_handler();//
                delay(1);//
            }
        }else if(act_info->active == UP){
            if(emj_run->emoji_mode){
                start_player();
                emj_run->emoji_mode = false;
                *timCont = millis();//记录开始播放的时间
            }
        } 
        act_info->active = ACTIVE_TYPE::UNKNOWN;
        act_info->isValid = 0;
    }
    free(timCont);
}
