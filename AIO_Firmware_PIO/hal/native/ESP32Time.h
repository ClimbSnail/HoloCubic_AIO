#ifndef ESP32TIME_H
#define ESP32TIME_H

#include <Arduino.h>
#include <time.h>
#include <sys/timeb.h>

class ESP32Time {
public:
    ESP32Time() : _offset(0) {}

    void setTime(long epoch = 1609459200, int ms = 0)
    {
        _offset = epoch - (long)time(NULL);
    }

    void setTime(int sc, int mn, int hr, int dy, int mt, int yr, int ms = 0)
    {
        struct tm t = {};
        t.tm_year = yr - 1900;
        t.tm_mon  = mt - 1;
        t.tm_mday = dy;
        t.tm_hour = hr;
        t.tm_min  = mn;
        t.tm_sec  = sc;
        time_t epoch = mktime(&t);
        setTime((long)epoch, ms);
    }

    tm getTimeStruct()
    {
        time_t now = time(NULL) + _offset;
        struct tm* t = gmtime(&now);
        return *t;
    }

    String getTime(String format)
    {
        struct tm t = getTimeStruct();
        char buf[32];
        strftime(buf, sizeof(buf), format.c_str(), &t);
        return String(buf);
    }

    String getTime()
    {
        char buf[16];
        struct tm t = getTimeStruct();
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
        return String(buf);
    }

    String getDateTime(bool mode = false)
    {
        struct tm t = getTimeStruct();
        char buf[32];
        if (mode) {
            snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                     t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                     t.tm_hour, t.tm_min, t.tm_sec);
        } else {
            snprintf(buf, sizeof(buf), "%02d/%02d/%04d %02d:%02d:%02d",
                     t.tm_mon + 1, t.tm_mday, t.tm_year + 1900,
                     t.tm_hour, t.tm_min, t.tm_sec);
        }
        return String(buf);
    }

    String getTimeDate(bool mode = false)
    {
        return getDateTime(mode);
    }

    String getDate(bool mode = false)
    {
        struct tm t = getTimeStruct();
        char buf[16];
        if (mode) {
            snprintf(buf, sizeof(buf), "%04d-%02d-%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
        } else {
            snprintf(buf, sizeof(buf), "%02d/%02d/%04d", t.tm_mon + 1, t.tm_mday, t.tm_year + 1900);
        }
        return String(buf);
    }

    String getDate(String format)
    {
        return getTime(format);
    }

    String getAmPm(bool lowercase = false)
    {
        struct tm t = getTimeStruct();
        const char* ampm = (t.tm_hour < 12) ? (lowercase ? "am" : "AM") : (lowercase ? "pm" : "PM");
        return String(ampm);
    }

    long getEpoch()
    {
        return (long)(time(NULL) + _offset);
    }

    long getMillis()
    {
        struct timeb tb;
        ftime(&tb);
        return (long)((time(NULL) + _offset) * 1000 + tb.millitm);
    }

    long getMicros()
    {
        struct timeb tb;
        ftime(&tb);
        return (long)((time(NULL) + _offset) * 1000000 + tb.millitm * 1000);
    }

    int getSecond()  { return getTimeStruct().tm_sec; }
    int getMinute()  { return getTimeStruct().tm_min; }
    int getHour(bool mode = false)
    {
        int h = getTimeStruct().tm_hour;
        if (mode) return h;
        return (h % 12 == 0) ? 12 : h % 12;
    }
    int getDay()          { return getTimeStruct().tm_mday; }
    int getDayofWeek()    { return getTimeStruct().tm_wday; }
    int getDayofYear()    { return getTimeStruct().tm_yday; }
    int getMonth()        { return getTimeStruct().tm_mon; }
    int getYear()         { return getTimeStruct().tm_year + 1900; }

private:
    long _offset;
};

#endif