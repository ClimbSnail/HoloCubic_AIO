#include "sys/app_controller.h"
#include "lvgl.h"
#include "stdlib.h"
#include "math.h"

/*
功能：赛博相册
操作说明：左倾停止自动切换、右倾恢复自动切换、后倾在静态和动态间切换
SD卡存放说明(cyber相关功能):
./LH&LXW/cyber/imgx.cyber       存放需要显示的图片文件(x为0~99)
./LH&LXW/cyber/cyber_num.txt    存放需要显示的图片文件数(00~99) 例如7个图片，写07

.cyber格式的图片文件由几行python程序生成，输入一张你要显示的图片(48x40)，就能得到对应的.cyber文件
python代码如下(python-v 3.8.16 openCV-v 4.7.0)：
import cv2
img_path = './123.jpg'#输入图片路径
out_path = './123.cyber'#输出文件路径
img = cv2.imread(img_path)
img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
with open(out_path, 'wb') as f:
    for a in img:
        for b in a:
            f.write(b)
*/


/* 系统变量 */
extern bool isCheckAction;
extern ImuAction *act_info;

#define cyber_play_time 2000//2000ms自动切换

struct cyber_run {
    bool flg;//true:当前屏幕显示pic1,false:当前屏幕显示pic2
    bool auto_play;//true:自动切换，false:不自动切换
    bool dynamic;//true:动态，false:静态
    bool str;//true:由"012<+="随机组成图片,false:只由'0'组成图片
    uint8_t cyber_num;//tf卡中图片总数量
    uint8_t cn;//当前播放的图片
    uint8_t py;//
    uint8_t con;//
    unsigned long timCon;
    uint8_t *testBuf;//屏幕单字节缓冲区
    uint8_t *pic1;//定义两张图片缓冲区，这样可以实现切换
    uint8_t *pic2;  
    File file;
};
cyber_run *cy_r=NULL;

/*释放此功能用的内存*/
void free_cy_r(void){
    free(cy_r->pic1);
    free(cy_r->pic2);
    free(cy_r->testBuf);
    free(cy_r);
}

/*屏幕缓冲区填充*/
static void testBuf_fill(uint8_t color){
    uint8_t i = 0;
    uint8_t j = 0;
    for (i = 0; i < 240; ++i)
        for (j = 0; j < 240; ++j)
            cy_r->testBuf[i * 240 + j] = color;
}

/* 5x6大小的字符，共6个 取模方式:阴码，列式，顺向，C51 */
const uint8_t font_scr[6][5]={
	0x38,0x44,0x44,0x38,0x00,
	0x00,0x44,0x7C,0x04,0x00,
	0x10,0x10,0x7C,0x10,0x00,//+
	0x4C,0x54,0x54,0x24,0x00,
	0x00,0x30,0x50,0x48,0x00,/*"<",7*/
	0x50,0x50,0x50,0x50,0x00,
};
/*在指定位置画指定颜色的字符(字符为font_scr[n])*/
// void df(uint8_t x,uint8_t y,uint8_t n,uint16_t color){
// 	uint8_t i=0,ii=0;
// 	uint8_t y0=y;
// 	for(;i<5;i++){
// 		for(ii=0;ii<6;ii++){
// 			if(font_scr[n][i] & (0x80>>ii))
// 				cy_r->testBuf[x+240*y] = color;
// 			y++;
// 		}
// 		y=y0;
// 		x++;
// 	}
// }
/*在指定位置画图片缓冲区中颜色的字符(字符为font_scr[n])*/
void dfpius(uint8_t x,uint8_t y,uint8_t n,const uint8_t *img){
	uint8_t i=0,ii=0;
	uint8_t y0=y;
	for(;i<5;y=y0,x++,x%=240,i++)//如果看不懂，同下
		for(ii=0;ii<6;y++,y%=240,ii++)//将第三个表达式 除i++外，其他的按从左至右的先后顺序依次放到for里面，就容易看懂一些
            if(font_scr[n][i] & (0x80>>ii))
                cy_r->testBuf[x+240*y] = map(img[y/6*48+x/5],0,255,0,7)<<2;		
}

/*此函数执行时长可以优化变的更快，只不过反正要延时，就没必要优化*/
void drawImg(void){
    testBuf_fill(0);
    uint8_t i=0, ii=0;
    for(;i<48;i++){
        for(ii=0;ii<40;ii++){
            if(cy_r->con<240){
                if((ii*6+cy_r->py)%240<cy_r->con)//从上至下切换
                // if(i*5<cy_r->con)//从左至右切换（可以实现上下左右四个方向的切换，这里只做从上至下的）
                    dfpius(i*5,(ii*6+cy_r->py)%240,rand()%(1+cy_r->str*5),cy_r->flg==true?cy_r->pic1:cy_r->pic2);
                else 
                    dfpius(i*5,(ii*6+cy_r->py)%240,rand()%(1+cy_r->str*5),cy_r->flg==true?cy_r->pic2:cy_r->pic1);                    
            }else
                dfpius(i*5,(ii*6+cy_r->py)%240,rand()%(1+cy_r->str*5),cy_r->flg==true?cy_r->pic1:cy_r->pic2);
        }
    }
    tft->pushImage(0, 0, 240, 240, cy_r->testBuf);//显示图像
    cy_r->py++;
    cy_r->py%=241;
    if(cy_r->con<240){//此时屏幕正在切换图片
        cy_r->con++;
        delay(9+rand()%14);
    }else if(cy_r->con!=245){//屏幕显示的图片切换完成，开始从tf卡更新此时没有显示的图片缓冲区(需要大概22.25ms左右)
        cy_r->cn++;
        if(cy_r->cn>cy_r->cyber_num)cy_r->cn=1;
        char *path = (char*)malloc(26);//必须用char*类型，不能用uint8_t*
        sprintf(path,"/LH&LXW/cyber/img%d.cyber",cy_r->cn);//图标路径
        cy_r->file = SD.open(path,"r");//建立File对象用于从SPIFFS中读取文件
        for(uint16_t i=0;i<1920;i++)
            if(cy_r->flg)cy_r->pic2[i] = cy_r->file.read();
            else cy_r->pic1[i] = cy_r->file.read();
        cy_r->file.close();//读取完后，关闭文件
        free(path);
        cy_r->con=245;
        cy_r->timCon = millis();//计时cyber_play_time毫秒
    }else{ 
        delay(9+rand()%14);
        if(millis()-cy_r->timCon > cyber_play_time && cy_r->auto_play){//计时cyber_play_time毫秒
            cy_r->flg = !cy_r->flg;
            cy_r->con = 0;
        }
    }
}

void cyber_pros(lv_obj_t *ym){

    cy_r = (cyber_run*)calloc(1,sizeof(cyber_run));
    if(cy_r == NULL){
        Serial.println("0:lack of memory");
        while(1);
    }  
    cy_r->testBuf = (uint8_t *)malloc(240 * 240); //动态分配一块屏幕分辨率大小的空间
    if(cy_r->testBuf == NULL){
        Serial.println("1:lack of memory");
        while(1);
    }  
    cy_r->pic1 = (uint8_t *)malloc(40*48); //动态分配一块图片大小的内存
    if(cy_r->pic1 == NULL){
        Serial.println("-1:lack of memory");
        while(1);
    }  
    cy_r->pic2 = (uint8_t *)malloc(40*48); //动态分配一块图片大小的内存
    if(cy_r->pic2 == NULL){
        Serial.println("1-:lack of memory");
        while(1);
    }
    cy_r->file = SD.open("/LH&LXW/cyber/cyber_num.txt","r");//建立File对象用于从SPIFFS中读取文件
    if(!cy_r->file){free_cy_r();return;}
    cy_r->cyber_num = (cy_r->file.read() - '0')*10;//读出图片数量
    cy_r->cyber_num += (cy_r->file.read() - '0');
    Serial.print("cyber_num:");Serial.println(cy_r->cyber_num);
    cy_r->file.close();//读取完后，关闭文件      

    /* 判断文件是否存在，如果都存在，此后打开就不需要判断了 */
    for(uint8_t i=1;i<=cy_r->cyber_num;i++){
        char *test_ = (char*)malloc(26);//必须用char*类型，不能用uint8_t*
        sprintf(test_,"/LH&LXW/cyber/img%d.cyber",i);//图标路径
        cy_r->file = SD.open(test_,"r");//建立File对象用于从SPIFFS中读取文件
        if(!cy_r->file){free_cy_r();return;}
        cy_r->file.close(); 
        free(test_);
    }

    cy_r->cn=1;//从第一张图片开始
    cy_r->con=0;
    cy_r->py=0;
    cy_r->auto_play = true;
    cy_r->flg = true;
    cy_r->dynamic = true;
    cy_r->str = true;

    /*从内存卡读取数据到图片缓冲区1*/
    cy_r->file = SD.open("/LH&LXW/cyber/img1.cyber","r");
    for(uint16_t i=0;i<1920;i++)
        cy_r->pic1[i] = cy_r->file.read();
    cy_r->file.close();//读取完后，关闭文件
    /*图片缓冲区2随机填充颜色*/
    for(uint16_t i=0;i<1920;i++)
        cy_r->pic2[i] = rand()%99;

    /*实现APP丝滑切换*/
    lv_obj_t *obj = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(obj,lv_color_hex(0),LV_STATE_DEFAULT);
    lv_scr_load_anim(obj, LV_SCR_LOAD_ANIM_OUT_BOTTOM, 673, 0, false);
    for(uint16_t i=0;i<873;i++){
        lv_timer_handler();//让LVGL更新屏幕，让操作者可以看到已执行动作
        delay(1);//
    }
    
//     testBuf_fill(0);
//     lv_scr_load_anim(ym, LV_SCR_LOAD_ANIM_OUT_TOP, 599, 0, false);//调用系统退出函数之前，一定要等待动画结束否则会导致系统重启
//     lv_obj_invalidate(lv_scr_act());//哪怕缓存没变，也让lvgl下次更新全部屏幕
//     /* 延时999ms，防止同时退出app */
//     for(uint16_t i=0;i<598+500;i++){
//         lv_timer_handler();//让LVGL更新屏幕，让操作者可以看到已执行动作
//         delay(1);//
//     }
//     lv_obj_clean(obj);
//     lv_obj_del(obj);
//     free(cy_r->testBuf);//释放57600字节内存
//     free(cy_r->pic2);//释放1920字节内存
//     free(cy_r->pic1);//释放1920字节内存
//     free(cy_r);
// return;

    while(1){
        if(cy_r->dynamic)
            drawImg();  
        act_info->active = ACTIVE_TYPE::UNKNOWN;
        act_info->isValid = 0;
        /* MPU6050数据获取 */
        if (isCheckAction){
            isCheckAction = false;
            act_info = mpu.getAction();
        }
        /* MPU6050动作响应 */
        if (RETURN == act_info->active){
            break;
        }else if(TURN_RIGHT == act_info->active){
            cy_r->str=!cy_r->str;
            delay(700);
        }else if(TURN_LEFT == act_info->active){
            cy_r->auto_play = !cy_r->auto_play;
            delay(700);
        }else if(act_info->active == UP){
            cy_r->dynamic=!cy_r->dynamic;
            if(cy_r->dynamic)
                for(uint8_t i=0;i<29;i++)
                    drawImg();  
            else delay(700);                
        } 
    }

    lv_scr_load_anim(ym, LV_SCR_LOAD_ANIM_OUT_TOP, 599, 0, false);//调用系统退出函数之前，一定要等待动画结束否则会导致系统重启
    lv_obj_invalidate(lv_scr_act());//哪怕缓存没变，也让lvgl下次更新全部屏幕
    /* 延时999ms，防止同时退出app */
    for(uint16_t i=0;i<598+500;i++){
        lv_timer_handler();//让LVGL更新屏幕，让操作者可以看到已执行动作
        delay(1);//
    }
    lv_obj_clean(obj);
    lv_obj_del(obj);

    
    free(cy_r->testBuf);//释放57600字节内存
    free(cy_r->pic2);//释放1920字节内存
    free(cy_r->pic1);//释放1920字节内存
    free(cy_r);

}
