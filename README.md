# HoloCubic
HoloCubic Firmware

* 原作者的项目链接 https://github.com/peng-zhihui/HoloCubic
* 本项目的地址 https://github.com/ClimbSnail/HoloCubic
* 或者 https://gitee.com/ClimbSnailQ/HoloCubic

### 本固件程序是基于前人的基础上修改了整体框架，更有利于多功能的实现
B站功能演示视频链接 https://www.bilibili.com/video/BV1jh411a7pV?from=search&seid=5347262882311444671

[^_^]:
	![HomePage](Image/holocubic_1080x1080.jpg)

![HomePage](https://gitee.com/ClimbSnailQ/Project_Image/raw/master/OtherProject/holocubic_1080x1080.jpg)

[^_^]:
	![HomePage](Image/holocubic_home.png)

![HomePage](https://gitee.com/ClimbSnailQ/Project_Image/raw/master/OtherProject/holocubic_home.png)

[^_^]:
	![UploadPage](Image/holocubic_upload.png)

![UploadPage](https://gitee.com/ClimbSnailQ/Project_Image/raw/master/OtherProject/holocubic_upload.png)

[^_^]:
	![SettingPage](Image/holocubic_setting.png)

![SettingPage](https://gitee.com/ClimbSnailQ/Project_Image/raw/master/OtherProject/holocubic_setting.png)

### 主要特点
1. 内置天气、时钟、相册（tf卡）、浏览器文件修改等功能。
2. 开机无论是否连接wifi（一定要2.4G的wifi），都不影响其他功能运行。
3. 程序相对模块化，低耦合。
4. 提供web界面进行配网以及其他设置选项。注：若当前模式为STA模式，则WebServer建立在STA模式下的Local_IP上。若为AP模式，则建立在AP_IP上（屏幕的服务界面有标注），AP模式的热点名为`HoloCubic_Pro`无密码。
5. 提供web端连入除了支持ip访问，也支持域名直接访问 http://holocubic
6. 提供web端的文件上传到SD卡（包括删除），无需拔插SD来更新图片。
7. 即使断网后，时钟也依旧运行。（开机最好连接wifi，这样会自动同步时钟。使用中会间歇尝试同步时钟）

### 功能切换说明：
1. TF卡的文件系统为fat32。在使用内存卡前最好将本工程中`放置到内存卡`目录里的所有文件和文件夹都放在TF卡的根目录。（以下会单独介绍tf卡里的配置文件）
2. 插不插tf内存卡都不影响开机，但影响wifi密码和相册照片的读取。
3. 左右摇晃即可切换界面。
4. 向前倾斜1s钟即可切换第二功能，今后还会整合更多功能，同样前倾1s即切换。

### TF卡文件说明
关于`wifi.txt`文件的结构一共5行。如果手动修改的需要注意每一行一定要加回车，**最后一行也必须有回车。**
1. wifi名称（一定要2.4G的wifi）
2. wifi密码
3. 所在省份（用于天气查询）
4. 上述省份的语言（默认使用zh-Hans 简体中文）
5. 知心天气的key（私钥）。（申请地址 https://seniverse.com ，文件里附带key是范例，无法直接使用。程序默认使用的是v3版本的api）

关于天气：程序启动后在天气的界面时，将会读取`weather/`目录下的图标文件。

关于图片转换：有空会出图片转换的工具。

### 固件更新：
根目录下的`firmware.bin`即为实现编译好的二进制固件文件，进行一定分区后方可上传。随后会出全套刷机教程。

### 之后计划
1. 添加视频播放。
2. 电脑桌面投屏。


### 硬件相关
**注意：硬件部分C7电容换成10uF左右就可以实现自动下载。**


### 关于编译工程代码
1. 本工程代码是基于vscode上的PlatformIO插件中的ESP32-Pic的Arduino平台开发。
2. 记得修改工程下`platformio.ini`文件中`upload_port`字段成对应自己COMM口。
3. 开发时，需要修改platformIO上对esp32的默认分区（否则编译大小超限，强制报错）。需要修改的文件为`.platformio/packages/framework-arduinoespressif32/boards.txt`，修改其中的`pico32.upload.maximum_size`字段的值为`2097152`（2M）够用就行。
4. 然后这里需要修改一个官方库文件才能正常使用：

首先非PlatformIO开发（自带包了）的用户需安装ESP32的Arduino支持包（百度有海量教程）。然后在安装的支持包的`esp32\hardware\esp32\1.0.4\libraries\SPI\src\SPI.cpp`文件中，**修改以下代码中的MISO为26**：

    if(sck == -1 && miso == -1 && mosi == -1 && ss == -1) {
        _sck = (_spi_num == VSPI) ? SCK : 14;
        _miso = (_spi_num == VSPI) ? MISO : 12; // 需要改为26
        _mosi = (_spi_num == VSPI) ? MOSI : 13;
        _ss = (_spi_num == VSPI) ? SS : 15;
这是因为，硬件上连接屏幕和SD卡分别是用两个硬件SPI，其中HSPI的默认MISO引脚是12，而12在ESP32中是用于上电时设置flash电平的，上电之前上拉会导致芯片无法启动，因此我们将默认的引脚替换为26。





