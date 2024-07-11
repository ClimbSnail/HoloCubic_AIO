/*****************************************************************************
File name: URLCode.h
Description: 适用于Arduino的URL编码解码
Author: Mr.Xie
Version: 1.0.0
Date: 2022/08/17
History: 
    * 0.1.0 测试版
    * 1.0.0 增加了看门狗代码补充模块 
      修改了 0.1.0 版本中 URLCode :: urlencode() 的部分错误  
      给#include "URLCode.cpp" 打上了注释 防止重复调用
*****************************************************************************/
#ifndef _URLCODE_H_
#define _URLCODE_H_

#include <Arduino.h>
#include <String.h>


class URLCode{
private:
    char dec2hex(short int c);
    int hex2dec(char c);
public:
    String urlcode; // URL 编码后
    String strcode; // URL 编码前
    void urlencode();   // 编码URL
    void urldecode();   // 解码URL
    void wdtFeed();     // 喂看门狗

};

//如果你不是直接下载到库里，而是复制到某个文件夹内，报错URLCode里面的函数没有定义的话，去掉下面这一行的注释
//#include "URLCode.cpp"

#endif








