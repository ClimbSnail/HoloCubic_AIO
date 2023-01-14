# HoloCubic_AIO (All in one for HoloCubic)
### _You can also read a translated version of this file [in English 英文版](https://github.com/ClimbSnail/HoloCubic_AIO/blob/main/README_English.md) or [in Korean 한국어](./README_English.md)._

`Holocubic`原为`稚晖君`大佬的开源项目，本项目则是运行在`HoloCubic硬件`之上的第三方独立固件项目。

AIO意为All in one，目的将尽可能多的功能集成进Holocubic AIO固件中，并坚持开源。目前固件源码达`2w+`行，上位机源码`4k`行（均不含字库图片），诚邀大家共同开发AIO固件、上位机以及周边，让固件满足更多人的需求。

此外，本固件代码完全开源，共大家学习、把玩，若使用本项目二次开源或部分参考，请适当注明参考来源。

* 原作者的项目链接 https://github.com/peng-zhihui/HoloCubic
* 本项目的地址 https://github.com/ClimbSnail/HoloCubic_AIO （最新版本）
* 或者 https://gitee.com/ClimbSnailQ/HoloCubic_AIO

### 开发人员
* AIO框架开发和部分APP编写：[ClimbSnail](https://github.com/ClimbSnail)
* 2048游戏应用开发：[AndyXFuture](https://github.com/AndyXFuture)
* 新版天气时钟应用开发：[PuYuuu](https://github.com/PuYuuu)
* BiliBili粉丝应用开发：[cnzxo](https://github.com/cnzxo/)
* 纪念日、心跳应用开发：[WoodwindHu](https://github.com/WoodwindHu)
* PC资源监控应用开发：[Jumping99](https://github.com/Jumping99)
* 开发人员持续加入中。。。。

_**欢迎加入AIO内测QQ讨论群**_

一群群号 755143193
二群群号 860112883
三群群号 676732387
四群群号 749207817

![QQ_Group](https://gitee.com/ClimbSnailQ/Project_Image/raw/master/OtherProject/holocubic_qq_group.jpg)

### 主要特点
1. 聚合多种APP，内置天气、时钟、相册、特效动画、视频播放、电脑投屏、web设置等等。（各APP具体使用参考说明书）
2. 开机无论是否插接tf卡、mpu6050是否焊接正常、是否连接wifi（一定要2.4G的wifi），都不影响系统启动和屏幕显示。
3. 程序相对模块化，低耦合。
4. 提供web界面进行配网以及其他设置选项。注：具体操作参考`APP介绍`
5. 提供web端连入除了支持ip访问，也支持域名直接访问 http://holocubic （部分浏览器可能支持不好）
6. 提供web端的文件上传到SD卡（包括删除），无需拔插SD来更新图片。
7. 提供全套上位机软件，并开源上位机源码。 https://github.com/ClimbSnail/HoloCubic_AIO_Tool

### 本固件设计了一套低耦合框架，更有利于多功能的实现
B站功能操作演示视频链接 https://www.bilibili.com/video/BV1wS4y1R7YF/

![HomePage](Image/holocubic_main.jpg)

![HomePage](Image/holocubic_home.png)

![UploadPage](Image/holocubic_upload.png)

![SettingPage](Image/holocubic_setting.png)


### 固件刷写工具（无需搭建任何IDE环境）
下载群中的上位机进行刷机。
1. `bootloader_dio_40m.bin`启动的`bootloader`。
2. `partitions.bin`分区文件
3. `boot_app0.bin`
4. 最新的固件`HoloCubic_AIO_XXX.bin`

其中`HoloCubic_AIO_XXX.bin`文件随着每次版本更新而更新，其他三个文件基本不会变动。

将以上前三个文件与`CubicAIO_Tool.exe`放在同一个目录下，双击运行`CubicAIO_Tool.exe`，软件中选择最新的固件`HoloCubic_AIO_XXX.bin`，即可刷写固件。

上位机操作演示视频链接 https://b23.tv/5e6uDh

本上位机与AIO固件一样，坚持开源，开源地址：https://github.com/ClimbSnail/HoloCubic_AIO_Tool

![AIO_TOOL](Image/holocubic_aio_tool.png)

![AIO_TOOL](https://gitee.com/ClimbSnailQ/Project_Image/raw/master/OtherProject/holocubic_aio_tool.png)

### 开机注意事项
由于小电视使用的是MPU6050陀螺仪加速度计，通电前3秒需要保持小电视自然放置（不要手拿），等待传感器初始化，初始化完毕后RGB灯会完全亮起，之后就可以正常操作了。插不插内存卡都不影响正常开机，如果6050焊接有问题，初始化后姿态读取会错乱（现象：应用会不断切换）。

### 功能切换说明：
1. TF卡的文件系统为fat32。TF为非必须硬件，但相册、视频播放等功能需依赖与此。如果准备使用内存卡，在使用内存卡前最好将本工程中`放置到内存卡`目录里的所有文件和文件夹都放在TF卡的根目录。
2. 插不插tf内存卡都不影响开机，但影响某些APP的功能（各自APP介绍里会说明）。
3. 左右摇晃`0.5s`即可切换选择各类APP。
4. 向前倾斜`1s`钟即可进入当前页的APP应用，今后还会整合更多功能，同样后仰1s即退出该APP。

### APP介绍

##### 网页配置服务（Web Server）
1. 运行条件：无。注：wifi等信息是保存在flash中，内存卡完全不影响wifi功能的连接。
2. 启用后，会显示`Web Sever Start`。小电视开启AP模式，建立在`AP_IP`上（屏幕的服务界面有标注），AP模式的热点名为`HoloCubic_AIO`无密码。
3. 开始使用时，应让电脑与`HoloCubic`处于同一网络环境（同网段）。如果之前没连接过wifi则需要使用电脑连接HoloCubic放出的热点名为`HoloCubic_AIO`无密码的wifi。
4. 在浏览器地址栏输入`Local_IP`或者`AP_IP`（ http://192.168.4.2 也支持域名直接访问 http://holocubic ），即可进入管理设置后台。推荐使用`ip地址`访问。
5. 网页里可设置系统参数、天气APP参数、相册参数、播放器参数等等。
6. 由于当前APP数量多，切换查找比较繁琐，也可以在`Web Server`中配置`自启动APP`。

##### 文件管理器（File Manager）
作用：通过无线网络管理内存卡上的文件。

1. 运行APP条件：必须是已经正常配置wifi。必须插内存卡。为避免wifi连接时，功率不够导致重启，请确保USB口供电充足。目前部分功能还在开发中。
2. 进入`Holocubic`文件管理器后会自动连接已配置的wifi，并显示出IP地址。
3. 未完成：在上位机的文件管理器软件中填入自己`Holocubic`的IP地址（端口可以不用改），点击连接。

注：目前文件管理器临时使用`windows资源管理器`，在地址栏输入 ftp://holocubic:aio@192.168.123.241 （192.168.123.241为我的小电视上显示的IP地址，如果提示开启访问，就开启）

##### 相册（Picture）
1. 运行APP条件：必须插内存卡，内存卡的根目录下必须存在`image/`目录（也可以使用`Web Server服务`APP 通过浏览器上传照片），`image/`目录下必须要有图片文件（jpg或者bin）。
2. 将需要播放的图片转化成一定格式（.jpg或.bin），再保存在`image/`目录中，图片文件名必须为英文字符或数字。
3. 使用固件进入相册APP后，将会读取`image/`目录下的图片文件。
4. `WebServer`的网页端可以进行附加功能的设置。

关于图片转换：使用附带的上位机转化（分辨率随意，软件会自动压缩到指定分辨率）。
* 常用的天气图片，转换为c数组，格式为Indexed 16 colors 选择 C array。
* 不常用的图片则可以转换成（True color with alpha 选择Binary RGB565）bin文件存储到SD卡中，这样可以省下一些程序存储空间用来增加功能。支持转化为jpg图片。

##### 视频播放（Media）
1. 运行APP条件：必须插内存卡，内存卡的根目录下必须存在`movie/`目录。
2. 将所需要播放的视频（最好长宽比是1:1），使用本固件配套的使用转化工具转化为目标文件（mjpeg或者rgb格式都可），存放在`movie/`目录下，视频文件名必须为英文字符或数字。
3. 运行播放器APP后，将会读取`movie/`目录下的视频文件。
4. 默认功率下，无任何动作90s后进入低功耗模式，120s后进入二级低功耗模式，具体表现为播放画面帧数下降。
5. `WebServer`的网页端可以进行附加功能的设置。

##### 屏幕分享、电脑投屏（Screen share）
1. 运行APP条件：无需内存卡，但需要利用`Web Server服务`app设置wifi密码（确保能连上路由器）。为避免wifi连接时，功率不够导致重启，请确保USB口供电充足。
2. 上位机目前使用第三方软件，后期会独立编写投屏上位机，提高性能。
3. 本投屏上位机使用的是[大大怪](https://gitee.com/superddg123/esp32-TFT/tree/master)的上位机，如果画面卡顿可以降低画质来提升帧数。
4. `WebServer`的网页端可以进行附加功能的设置。

##### 天气、时钟（Weather）
一共有两款天气时钟APP
###### 新版 weather
1. 新版由`PuYuuu`模仿了`misaka`的时钟界面。使用https://www.tianqiapi.com 天气API。
2. 运行APP条件：必须是已经联网状态，且设置好`tianqi_appid`、`tianqi_appsecret`、`tianqi 城市名（中文）`。
2. 使用新版天气时钟，需要再"Web Server"网页服务中修改`tianqi_appid`、`tianqi_appsecret`。（申请地址 https://www.yiketianqi.com/user/login ）
###### 旧版 weather old
1. 旧版UI设计模仿了[CWEIB](https://github.com/CWEIB)
2. 运行APP条件：必须是已经联网状态，且设置好地点、weather_key。不插内存卡大多数情况能正常工作。
3. 一般情况下不插内存卡也可以工作，但部分天气图标是存在内存卡中（由于内部flash不够用）的，需要将固件附带的`weather/`文件夹复制到tf卡根目录。
4. 使用旧版天气时钟，需要再"Web Server"网页服务中修改心知天气的`心知天气 城市名`、`key（私钥）`。（申请地址 https://seniverse.com 。程序默认使用的是v3版本的api）

注：即使断网后，时钟也依旧运行。（开机最好连接wifi，这样会自动同步时钟。使用中会间歇尝试同步时钟）

##### 特效动画（Idea）
1. 运行APP条件：无。内置的几种特效动画。

注：移植群友"小飞侠"的功能，在此感谢！

##### 2048 APP
1. `2048`游戏由群友`AndyXFuture`编写并同意，由`ClimbSnail`合入AIO固件。原项目链接为`https://github.com/AndyXFuture/HoloCubic-2048-anim`
2. 运行APP条件：无。基本屏幕能亮就行。
3. 操作注意：游戏中`向上`和`向下`操作由于与原`进入`和`退出`为同一个动作，系统已操作时长为区分动作，游戏中`向上`和`向下`正常操作即可，`进入`和`退出`需要倾斜1秒中方可触发。

##### BiliBili APP
1. 运行APP条件：内存卡中必须要有名为`bilibili`的文件夹。必须是已经正常配置wifi。为避免wifi连接时，功率不够导致重启，请确保USB口供电充足。
2. `UID`查看方法：电脑浏览器上打开B站并登入账号，之后浏览器打开一个空白页粘贴回车这个网址 https://space.bilibili.com/ ，网址尾巴会自动多出一串纯数字码，此即为UID。
3. 第一次使用之前，要先在`WebServer App`的网页上填写`UID`码。
4. 需要在内存卡中名为`bilibili`的文件夹里添加一张名为`avatar.bin`自己B站头像的图片，分辨率为`100*100`的`bin`文件（可以使用AIO上位机转换）。

注：程序由`cnzxo`编写。

##### 纪念日（Anniversary）
1. 运行APP条件：联网状态
2. 第一次使用之前，要先在`WebServer App`的网页上填写纪念日名称和日期，目前可以设置两个纪念日。纪念日支持的字有`生日还有毕业养小恐龙种土豆老婆女朋友爸妈爷奶弟妹兄姐结婚纪念`，如果纪念日名称包含的字不在这个范围内，请自行生成字体文件并替换`src\app\anniversary\msyhbd_24.c`文件。日期格式如`2022.5.8`，如果年份设置为0，则被认为是每年重复的纪念日（如生日）。

注：纪念日和心跳都复现自[LizCubic](https://github.com/qingehao/LizCubic)项目。程序由`WoodwindHu`编写

##### 心跳（Heartbeat）
1. 运行APP条件：联网状态（需要开启性能模式），一个开放1883端口的mqtt服务器，两个HoloCubic。
2. 第一次使用之前，要先在`WebServer App`的网页上填写配置。role可以选择0和1，分别代表互动的两个HoloCubic。client_id为设备的唯一标识，这里请将这两个Holocubic设置成同一个QQ号。mqtt_server填写自己的mqtt服务器地址,port填写端口号。用户名以及密码根据具体的服务器配置而定。
3. 设置完心跳APP之后，开机自动联网，并开启mqtt客户端。收到另一个HoloCubic的消息之后自动进入APP。正常方式进入APP则自动向另一个HoloCubic发送消息。
4. 群内不定时更新免费的服务，具体配置参数可以问管理或者群友。

注：纪念日和心跳都复现自[LizCubic](https://github.com/qingehao/LizCubic)项目。程序由`WoodwindHu`编写

##### 股票行情实时查看（Stock）
1. 运行APP条件：必须是已经正常配置wifi。为避免wifi连接时，功率不够导致重启，请确保USB口供电充足。
2. 第一次使用之前，要先在`WebServer App`的网页上修改自己想要的`股票代码`。

注：程序由`redwolf`编写

##### PC资源监控(PC Resource)
1. 运行条件: 必须是已经正常配置wifi。PC端与HoloCubic处于同一网段，在`WebServer APP`中这只PC的服务IP地址（具体看教程）。
2. 下载[AIDA64](https://www.aida64.com/downloads)，PC安装AIDA64后的导入配置文件`aida64_setting.rslcd`（在`AIO_Firmware_PIO\src\app\pc_resource`目录下或者群文件中）

注：具体操作步骤较长，见群文档。本应用由`Jumping99`开发。

### 关于编译工程代码
1. 本工程代码是基于vscode上的PlatformIO插件中的ESP32-Pico的Arduino平台开发。具体教程可以上`B站`找。推荐教程[https://b23.tv/kibhGD](https://b23.tv/kibhGD)
2. 记得修改工程下`platformio.ini`文件中`upload_port`字段成对应自己COMM口。
3. 如果希望屏蔽掉内置的部分文件，请在`AIO_Firmware_PIO\src\app\app_conf.h`中对应的`APP宏`修改为`0`即可屏蔽。
4. 目前最新版本已经不需要大家特意修改SPI库中的SPI引脚了，本工程的`lib`下单独放置了一个已经修改好了SPI引脚的SPI库。以下是此前版本的操作（可忽略）

~~然后这里需要修改一个官方库文件才能正常使用(不然会导致内存卡读取失败)：
PlatformIO和ArduinoIDE用户都需安装ESP32的Arduino固件支持包（百度有海量教程）。不管哪种开发方式都需要修改`SPI`库中的`MISO`默认引脚为`26`，例如arduinoIDE的包路径为`esp32\hardware\esp32\1.0.4\libraries\SPI\src\SPI.cpp`文件中，**修改以下代码中的MISO为26**：~~
```
    if(sck == -1 && miso == -1 && mosi == -1 && ss == -1) {
        _sck = (_spi_num == VSPI) ? SCK : 14;
        _miso = (_spi_num == VSPI) ? MISO : 12; // 需要改为26
        _mosi = (_spi_num == VSPI) ? MOSI : 13;
        _ss = (_spi_num == VSPI) ? SS : 15;
```
~~这是因为，硬件上连接屏幕和SD卡分别是用两个硬件SPI，其中HSPI的默认MISO引脚是12，而12在ESP32中是用于上电时设置flash电平的，上电之前上拉会导致芯片无法启动，因此我们将默认的引脚替换为26。~~

### 程序框架图

![HoloCubic_AIO_Frame](Image/holocubic_AIO_Frame.png)

![HoloCubic_AIO_Frame](https://gitee.com/ClimbSnailQ/Project_Image/raw/master/OtherProject/holocubic_AIO_Frame.png)

AIO框架讲解链接 https://www.bilibili.com/video/BV1jh411a7pV?p=4

关于UI的设计可以自行关注下`Edgeline
`、`gui-guider`等工具。

platformIO模拟器 https://github.com/lvgl/lv_platformio

应用图标(128*128)：可以下载阿里矢量图 https://www.iconfont.cn/

调试过程中报错定位代码：`xtensa-esp32-elf-addr2line -pfiaC -e 固件名.elf Backtrace地址信息`

关于lvgl的学习参考：`http://lvgl.100ask.org`、`http://lvgl.100ask.net`

lvgl的字体生成可以使用：`LvglFontTool V0.4` 已放置在Doc目录下

c文件中提取所有汉字：可以使用工程下的`Script/get_font.py`脚本提取。`python get_font.py 字模.c文件的路径`

### 致谢
* ESP32内存分布 https://blog.csdn.net/espressif/article/details/112956403
* 视频播放 https://github.com/moononournation/RGB565_video
* FTP参考文档 https://blog.csdn.net/zhubao124/article/details/81662775
* ESP32 arduino运行双核 https://www.yiboard.com/thread-1344-1-1.html
* 强制门户认证 https://blog.csdn.net/xh870189248/article/details/102892766
* 感谢`lib`目录下所用到的开源库相关的作者

