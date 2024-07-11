#ifndef _URLCODE_CPP_
#define _URLCODE_CPP_

#include "URLCode.h"

int URLCode :: hex2dec(char c){
    if ('0' <= c && c <= '9') 
    {
        return c - '0';
    } 
    else if ('a' <= c && c <= 'f')
    {
        return c - 'a' + 10;
    } 
    else if ('A' <= c && c <= 'F')
    {
        return c - 'A' + 10;
    } 
    else 
    {
        return -1;
    }
}


char URLCode :: dec2hex(short int c)
{
    if (0 <= c && c <= 9) 
    {
        return c + '0';
    } 
    else if (10 <= c && c <= 15) 
    {
        return c + 'A' - 10;
    } 
    else 
    {
        return -1;
    }
}


// Chinese 编码一个url
// English Encode URLCode
void URLCode :: urlencode()
{
    urlcode = "";
    int i = 0;
    int len = strcode.length();
    for (i = 0; i < len; ++i) 
    {
        wdtFeed();
        char c = strcode[i];
        if (    ('0' <= c && c <= '9') ||
                ('a' <= c && c <= 'z') ||
                ('A' <= c && c <= 'Z') || 
                c == '/' || c == '.') 
        {
            urlcode += String(c);
        } 
        else 
        {
            int j = (short int)c;
            if (j < 0)
                j += 256;
            int i1, i0;
            i1 = j / 16;
            i0 = j - i1 * 16;
            urlcode += String('%');
            urlcode += String((char)dec2hex(i1));
            urlcode += String((char)dec2hex(i0));
        }
    }
}

// Chinese 解码url
// English Decode URLCode
void URLCode :: urldecode()
{
    strcode = "";
    int i = 0;
    int len = urlcode.length();
    for (i = 0; i < len; ++i) 
    {
        wdtFeed();
        char c = urlcode[i];
        if (c != '%') {
            strcode += String(c);
        } else {
            char c1 = urlcode[++i];
            char c0 = urlcode[++i];
            int num = 0;
            num = hex2dec(c1) * 16 + hex2dec(c0);
            strcode += String((char)num);
        }
    }
}

// Chinese 喂看门狗
// English Feed Watchdog)
void URLCode :: wdtFeed(){
    // 喂看门狗代码 如果需要解码的URL太过长，会导致芯片以为运行出错，所以需要进行喂狗 
    // 如果是 ESP8266 只需要 #define ESP8266 即可 其他芯片的喂狗代码请自行添加

    #ifdef ESP8266
    ESP.wdtFeed();
    #endif

}


#endif
