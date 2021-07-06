# HoloCubic_AIO (All in one for HoloCubic)


* 原作者的项目链接 https://github.com/peng-zhihui/HoloCubic
* 本项目的地址 https://github.com/ClimbSnail/HoloCubic_AIO （最新版本）
* 或者 https://gitee.com/ClimbSnailQ/HoloCubic_AIO

_**欢迎加入QQ讨论群 755143193**_

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
4. 提供web界面进行配网以及其他设置选项。注：若当前模式为STA模式，则WebServer建立在STA模式下的Local_IP上。若为AP模式，则建立在AP_IP上（屏幕的服务界面有标注），AP模式的热点名为`HoloCubic_AIO`无密码。
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

关于图片转换：有空会出图片转换的工具。目前先自行手动转化(尺寸240*240)，常用的天气图片利用lvgl的官方转换器 https://lvgl.io/tools/imageconverter 转换为c数组，格式为Indexed 16 colors。不常用的图片则可以转换成bin文件存储到SD卡中，这样可以省下一些程序存储空间用来增加功能。

应用图标：可以下载阿里矢量图 https://www.iconfont.cn/

### 固件更新：
根目录下的`HoloCubic_AIO.bin`即为事先编译好的二进制固件文件，进行一定分区后方可上传。随后会出全套刷机教程。
##### 关于烧录
对ESP32进行开发，将代码编译好了之后，可以提取后在其他设备使用ESP Flash Download Tool直接烧录。使用ESP Flash Download Tool烧录需要提前准备四个文件，其中包含两个启动引导文件`bootloaderbin`、`boot_app0.bin`，一个flash划分文件`partitions.bin`和一个固件文件`firmware.bin`当然名字是可以更改的。

下面说下这些文件的存放位置以及烧录地址：

以Windows为例()
1. `bootloader.bin`的位置为PlatformIO安装目录下的`.platformio\packages\tool-esptoolpy\test\image`目录下面,它的对应的烧录地址为0X1000。
2. `boot_app0.bin`的位置为PlatformIO安装目录下的`platformio\packages\framework-arduinoespressif32\tools\partitions`目录下面，它对应的烧录地址为0xe000
3. `partitions.bin`的位置为代码工程目录下的.pioenvs\[board]目录下面,它对应的烧录地址为0x8000。同时platformio\packages\framework-arduinoespressif32\tools\partitions目录下面的`partitions.csv`为编译的分区配置文件，会根据版型选择的不同有所不同，可以使用Excel打开进行编辑，然后在编译器内使用PIO进行重新编译即可，同时他也可以使用PIO包里面带的`gen_esp32part.py`脚本进行编译与反编译，操作方法为：python C:\SPB_Data\.platformio\packages\framework-arduinoespressif32\tools\gen_esp32part.py --verify xxx.csv xxx.bin(后面填写csv文件或者bin文件存放的位置，这里是将csv转换成bin，如果将位置对换，则可以将bin转换成csv)
4. firmware.bin的位置为代码工程目录下的.pioenvs\[board]目录下面，这个就是代码编译出来的固件，它对应的烧录地址为0x10000，如果分区文件未做修改的话（人为修改，或者更换编译平台），更新固件或者重新烧录只在对应地址开始需要烧录这一个文件即可。

同上面介绍的对分区文件的相互转换的操作方法相似的烧录方法为：

`python C:\SPB_Data\.platformio\packages\tool-esptoolpy\esptool.py --port COM19 --baud 115200 write_flash -fm dio -fs 4MB 0x010000 xxx\firmware.bin`后面选择编译出来固件，进行代码更新，直接可以在cmd.exe内执行这条指令即可，方便快捷。

##### 烧录参考脚本
1. python tool-esptoolpy\esptool.py --port COM7 --baud 115200 write_flash -fm dio -fs 4MB 0x1000 bootloader_esp32c3.bin
2. python tool-esptoolpy\esptool.py --port COM7 --baud 115200 write_flash -fm dio -fs 4MB 0x8000 boot_app0.bin
3. python tool-esptoolpy\esptool.py --port COM7 --baud 115200 write_flash -fm dio -fs 4MB 0xe000 partitions.bin
4. python tool-esptoolpy\esptool.py --port COM7 --baud 115200 write_flash -fm dio -fs 4MB 0x00010000 HoloCubic_AIO固件_v1.3.bin

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





