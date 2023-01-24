#ifndef APP_MOOD_H
#define APP_MOOD_H

#include "sys/interface.h"

#define MOOD_APP_NAME "Mood"

extern APP_OBJ mood_app;


//开始播放视频
static void video_start();
//释放播放器对象
static void release_player_decoder();
//更新心情
static int update_mood();

#endif