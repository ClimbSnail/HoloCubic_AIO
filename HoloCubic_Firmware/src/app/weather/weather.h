#ifndef APP_WEATHER_H
#define APP_WEATHER_H

#include "sys/interface.h"

struct Weather
{
    int weather_code;
    int temperature;
};

extern APP_OBJ weather_app;

#endif