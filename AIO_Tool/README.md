## 项目为AIO固件配套上位机
Holocubic_AIO开源地址 https://github.com/ClimbSnail/HoloCubic_AIO

[^_^]:
	![AIO_TOOL](Image/holocubic_aio_tool.png)

![AIO_TOOL](https://gitee.com/ClimbSnailQ/Project_Image/raw/master/OtherProject/holocubic_aio_tool.png)

## 重要问题
本工程包含了上位机所有代码及资源文件，但唯独缺少视频转化工具`ffmpeg`（文件太大），需要转化功能的可以自行访问`ffmpeg`原项目地址 https://github.com/FFmpeg/FFmpeg 下载，把其中的`ffmpeg.exe`文件放在本工程的根目录下即可。


### 打包成可执行程序
使用pyinstaller打包程序`pyinstaller --icon ./holo_256.ico -w -F CubicAIO_Tool.py`

## 开发笔记

#### 关于烧录
对ESP32进行开发完后，烧录需要提取四个文件，其中包含两个启动引导文件`bootloader_dio_80m.bin`、`boot_app0.bin`，一个flash划分文件`partitions.bin`和一个固件文件`firmware.bin`(在本工程里名为`HoloCubic_AIO固件_vX.X.X.bin`)。https://github.com/ClimbSnail/HoloCubic_AIO/releases/tag/v2.1.0%E5%9B%BA%E4%BB%B6


###### 下面说下这些文件的存放位置以及烧录地址（以Windows为例）：
1. `bootloader_dio_80m.bin`的位置为PlatformIO安装目录下的`.platformio\packages\framework-arduinoespressif32\tools\sdk\bin`目录下面,它的对应的烧录地址为0X1000。
2. `boot_app0.bin`的位置为PlatformIO安装目录下的`platformio\packages\framework-arduinoespressif32\tools\partitions`目录下面，它对应的烧录地址为0xe000
3. `partitions.bin`的位置为代码工程目录下的.pioenvs\[board]目录下面,它对应的烧录地址为0x8000。同时platformio\packages\framework-arduinoespressif32\tools\partitions目录下面的`partitions.csv`为编译的分区配置文件，会根据版型选择的不同有所不同，可以使用Excel打开进行编辑，然后在编译器内使用PIO进行重新编译即可，同时他也可以使用PIO包里面带的`gen_esp32part.py`脚本进行编译与反编译，操作方法为：python C:\SPB_Data\.platformio\packages\framework-arduinoespressif32\tools\gen_esp32part.py --verify xxx.csv xxx.bin(后面填写csv文件或者bin文件存放的位置，这里是将csv转换成bin，如果将位置对换，则可以将bin转换成csv),它的对应的烧录地址为0X8000。
4. `firmware.bin`的位置为代码工程目录下的`.pioenvs\[board]`目录下面，这个就是代码编译出来的固件，它对应的烧录地址为0x10000，如果分区文件未做修改的话（人为修改，或者更换编译平台），更新固件或者重新烧录只在对应地址开始需要烧录这一个文件即可。此文件手动命名为`HoloCubic_AIO固件_vX.X.X.bin`，由于经常随着源码的更新而更新。,它的对应的烧录地址为0x10000。

#### 烧录参考脚本
1. python tool-esptoolpy\esptool.py --port COM7 --baud 921600 write_flash -fm dio -fs 4MB 0x1000 bootloader_dio_80m.bin 0x00008000 partitions.bin 0x0000e000 boot_app0.bin 0x00010000 HoloCubic_AIO固件_v1.3.bin
2. 清空flash命令 python tool-esptoolpy\esptool.py erase_flash

可用波特率为：
* 115200
* 230400
* 460800
* 576000
* 921600
* 1152000


#### 图片转化开发要点
https://lvgl.io/assets/images/logo_lvgl.png

利用lvgl的官方转换器 https://lvgl.io/tools/imageconverter 图片则可以转换成（True color with alpha 选择Binary RGB565）bin文件存储到SD卡中

## 致谢
* 固件下载工具 https://github.com/espressif/esptool
* 视频转码工具 https://github.com/FFmpeg/FFmpeg
* LVGL离线转换工具 https://github.com/W-Mai/lvgl_image_converter

