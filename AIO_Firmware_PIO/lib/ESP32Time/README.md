# ESP32Time
An Arduino library for setting and retrieving internal RTC time on ESP32 boards

[![arduino-library-badge](https://www.ardu-badge.com/badge/ESP32Time.svg?)](https://www.ardu-badge.com/ESP32Time)

## Functions

```
setTime(30, 24, 15, 17, 1, 2021);  // 17th Jan 2021 15:24:30
setTime(1609459200);  // 1st Jan 2021 00:00:00
setTime();            // default 1st Jan 2021 00:00:00

getTime()          //  (String) 15:24:38
getDate()          //  (String) Sun, Jan 17 2021
getDate(true)      //  (String) Sunday, January 17 2021
getDateTime()      //  (String) Sun, Jan 17 2021 15:24:38
getDateTime(true)  //  (String) Sunday, January 17 2021 15:24:38
getTimeDate()      //  (String) 15:24:38 Sun, Jan 17 2021
getTimeDate(true)  //  (String) 15:24:38 Sunday, January 17 2021

getMicros()        //  (long)    723546
getMillis()        //  (long)    723
getEpoch()         //  (long)    1609459200
getSecond()        //  (int)     38    (0-59)
getMinute()        //  (int)     24    (0-59)
getHour()          //  (int)     3     (0-12)
getHour(true)      //  (int)     15    (0-23)
getAmPm()          //  (String)  pm
getAmPm(true)      //  (String)  PM
getDay()           //  (int)     17    (1-31)
getDayofWeek()     //  (int)     0     (0-6)
getDayofYear()     //  (int)     16    (0-365)
getMonth()         //  (int)     0     (0-11)
getYear()          //  (int)     2021

getTime("%A, %B %d %Y %H:%M:%S")   // (String) returns time with specified format 
```
[`Formatting options`](http://www.cplusplus.com/reference/ctime/strftime/)
