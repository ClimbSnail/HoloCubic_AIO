#ifndef _APP_CONF_H
#define _APP_CONF_H

/**
 * 将某项APP对应的宏定义设置为1，可以将对应APP的
 * 代码编译进固件，设置为0则不编译进固件。便于腾
 * 出额外的flash空间给其他用户自定义app。
 * 注: 当某个app不使能时，对应的web配置项也不会
 * 出现在配置网页上。
*/

//在开发新的app时，可以先把其他所有APP关闭，这样烧录速度也比较快

//纪念日APP
#define APP_ANNIVERSARY_USE 0 

//bilibili粉丝APP
#define APP_BILIBILI_FANS_USE 1

//文件管理器APP
#define APP_FILE_MANAGER_USE 0

//2048游戏APP
#define APP_GAME_2048_USE 0

//心跳APP
#define APP_HEARTBEAT_USE 0

//特效动画APP
#define APP_IDEA_ANIM_USE 0

//视频播放器APP
#define APP_MEDIA_PLAYER_USE 1

//相册APP
#define APP_PICTURE_USE 1

//遥感器APP
#define APP_REMOTE_SENSOR_USE 1

//屏幕分享/投屏APP
#define APP_SCREEN_SHARE_USE 0

//网页配置服务APP
//此项用于配置小电视，为必选项，此处仅为了说明有这个app，此宏无作用
#define APP_WEB_SERVER_USE       

//设置APP
#define APP_SETTING_USE 0

//股票行情查看APP
#define APP_STOCK_MARKET_USE 0

//天气APP
//如flash空间不够，首选配置天气app二选一，腾出大量flash空间
#define APP_WEATHER_USE 1

//天气(旧)APP
//如flash空间不够，首选配置天气app二选一，腾出大量flash空间
#define APP_WEATHER_OLD_USE 0

#endif
