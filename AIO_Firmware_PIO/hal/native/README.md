# Native 仿真框架

在 Windows PC 上完整仿真 ESP32 固件的运行环境，支持 FreeRTOS 多任务调度、LVGL 图形界面、网络通信、文件系统和自动测试，无需真实硬件即可开发和调试。

## 目录

- [环境配置](#环境配置)
- [编译运行](#编译运行)
- [按键映射](#按键映射)
- [文件系统映射](#文件系统映射)
- [自动测试](#自动测试)
- [架构概览](#架构概览)
- [常见问题](#常见问题)

---

## 环境配置

### 1. 安装 MSYS2

下载并安装 [MSYS2](https://www.msys2.org/)，默认安装路径为 `C:\msys64`。

### 2. 安装编译工具链和 SDL2

打开 MSYS2 MinGW64 终端，执行：

```sh
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2
```

### 3. 安装 PlatformIO

推荐通过 VS Code 扩展安装 PlatformIO IDE，或使用 pip：

```sh
pip install platformio
```

### 4. SDL2 环境变量（可选）

若 MSYS2 安装在非默认路径，需将 `mingw64/bin` 添加到系统 PATH 环境变量。

---

## 编译运行

### 编译命令

在项目根目录下打开 PowerShell，执行：

```powershell
TaskKill /F /IM program.exe 2>$null
C:\msys64\msys2_shell.cmd -mingw64 -defterm -no-start -c "cd /d/<你的路径>/HoloCubic_AIO/AIO_Firmware_PIO && platformio run -e AIO_native_sim > build_log.txt 2>&1; grep -E 'Compiling|error:|Linking|SUCCESS|FAILED' build_log.txt"
```

> **注意**：将 MSYS2 路径和项目路径替换为你的实际路径。如果 PlatformIO 安装在默认位置，MSYS2 内可直接使用 `platformio` 命令。

### 编译产物

编译成功后在 `AIO_Firmware_PIO/.pio/build/AIO_native_sim/program.exe` 生成可执行文件。

### 运行

```powershell
cd AIO_Firmware_PIO
.\pio\build\AIO_native_sim\program.exe
```

### 启用/禁用 APP

编辑 `AIO_Firmware_PIO/platformio.ini`，在 `[env:AIO_native_sim]` 节中：

- `-D APP_XXX_USE=1` 启用某个 APP
- `-D APP_XXX_USE=0` 禁用某个 APP
- `build_src_filter` 中 `-<src/app/xxx/>` 排除对应 APP 源码

> ⚠️ `APP_XXX_USE` 宏和 `build_src_filter` 是配对开关，必须同步修改。

---

## 按键映射

键盘按键模拟硬件 MPU6050 姿态操作：

| 硬件操作 | 键盘按键 | 说明 |
|---------|---------|------|
| 左倾 | ← (Left) | 切换上一个 APP |
| 右倾 | → (Right) | 切换下一个 APP |
| 前倾 | Space | 进入 APP / 确认 |
| 后仰 | Backspace | 退出 APP / 返回 |
| 晃动 | Enter | 摇晃设备 |

---

## 文件系统映射

仿真框架将固件的虚拟文件系统映射到本地目录：

### SPIFFS（闪存文件系统）

SPIFFS 操作映射到 `sim_data/spiffs/` 目录（相对于 `program.exe` 所在目录）。

`sim_data/spiffs/` 目录结构：
```
sim_data/spiffs/
├── config.cfg        ← 系统配置文件
├── holocubic_aio.cfg ← AIO 配置文件
├── weather.cfg       ← 天气配置
├── tomato.cfg        ← 番茄钟配置
└── ...
```

### SD 卡

SD 卡操作映射到 `sim_data/sd/` 目录。

`sim_data/sd/` 目录结构：
```
sim_data/sd/
├── image/            ← 相册图片
├── movie/            ← 视频文件
├── weather/          ← 天气图标 (.bin)
├── bilibili/         ← B站头像
├── LH&LXW/           ← 多功能动画资源
└── ...
```

### sim_data 目录位置

`sim_data/` 目录位于项目根目录（与 `AIO_Firmware_PIO/` 同级），已包含基本的配置文件和应用资源。首次使用无需额外配置，直接编译运行即可。

---

## 自动测试

仿真框架内置自动测试系统，可通过命令行参数或环境变量运行。

### 运行单个测试

```powershell
cd AIO_Firmware_PIO
$env:AUTO_TEST="File Manager"
.\pio\build\AIO_native_sim\program.exe
```

### 已注册测试用例

| 测试名称 | 步骤数 | 说明 |
|---------|:-----:|------|
| Picture | 9 | 相册 APP |
| 2048 | 11 | 2048 游戏 |
| Settings | 1 | 设置 APP |
| Idea | 4 | 特效动画 |
| WebServer | 3 | 网页配置服务 |
| File Manager | 7 | 文件管理器 |
| LH&LXW | 20 | 多功能动画（子应用切换） |
| Stock | 4 | 股票行情 |
| Anniversary | 4 | 纪念日 |
| Bili | 4 | B站粉丝 |
| Heartbeat | 3 | 心跳 |
| Tomato | 4 | 番茄钟 |
| Weather | 4 | 新版天气 |
| Weather Old | 4 | 旧版天气 |
| Snake | 4 | 贪吃蛇 |
| PC Resource | 3 | PC资源监控 |
| Screen share | 3 | 屏幕投屏 |
| Media | 8 | 视频播放器 |

### 新增测试用例

在 `hal/native/auto_test.cpp` 中：

1. 定义测试步骤函数
2. 在 `test_cases[]` 数组中新增条目：`{"APP名称", 导航索引, 步骤函数, 步骤数, 每步超时帧数}`
3. 重新编译

---

## 架构概览

### 目录结构

```
hal/native/
├── sim_main.cpp          ← 仿真入口（替换原 HoloCubic_AIO.cpp）
├── hal_display.c         ← SDL2 显示驱动（240×240 窗口）
├── hal_imu.c             ← 键盘模拟 MPU6050 姿态
├── hal_bh1750.c          ← BH1750 环境光传感器模拟
├── hal_sd.c              ← SD 卡模拟
├── hal_rgb.c             ← RGB LED 模拟
├── hal_native.h          ← HAL 主头文件
├── auto_test.cpp/h       ← 自动测试框架
├── log_buffer.cpp/h      ← 异步日志缓冲（环形缓冲 + 消费者线程）
├── serial_utils.c/h      ← 带时间戳调试日志
├── network_bridge.cpp    ← WinSock2 网络桥接
├── sim_data_path.cpp/h   ← sim_data 路径解析
├── lv_fs_sim.cpp         ← LVGL 仿真 S: 驱动（SD 卡文件系统）
├── driver_stubs.cpp      ← 驱动桩函数实现
├── FreeRTOSConfig.h      ← FreeRTOS 仿真配置
├── freertos/semphr.h     ← 信号量 shim
├── stb_image.h           ← JPEG 解码库（单头文件）
│
├── Arduino.h/cpp         ← Arduino 环境桩
├── FS.h, SD.h/cpp        ← 文件系统桩
├── SPIFFS.h/cpp          ← SPIFFS 桩
├── SPI.h/cpp, Wire.h/cpp ← 总线桩
├── TFT_eSPI.h/cpp        ← 显示驱动桩
├── WiFi.h/cpp, HTTPClient.h, WebServer.h/cpp  ← 网络桩
├── MPU6050.h, I2Cdev.h   ← 传感器桩
├── TJpg_Decoder.h/cpp    ← JPEG 解码桩
├── ff.h/cpp              ← FatFS 桩
└── ...                   ← 其他 ESP32 库桩文件
```

### 仿真隔离原则

- `src/driver/` 为实际固件驱动，**禁止修改**
- 所有仿真代码放在 `hal/native/`，通过 `platformio.ini` 的 `build_src_filter` 排除原驱动
- 不确定文件归属时，先查 `platformio.ini` 的 `build_src_filter` 列表

### 技术栈

| 组件 | 技术 |
|------|------|
| 编译环境 | MSYS2 MinGW64 (GCC) |
| 构建系统 | PlatformIO (`env:AIO_native_sim`) |
| 图形显示 | SDL2 (240×240 窗口) |
| 网络 | WinSock2（真实 TCP/IP） |
| RTOS | FreeRTOS-Kernel (Win32 port) |
| UI 框架 | LVGL v8.3 |
| 图像解码 | stb_image |

---

## 常见问题

### Q: 编译报 "SDL2/SDL.h not found"

MSYS2 中未安装 SDL2 包，执行 `pacman -S mingw-w64-x86_64-SDL2`。

### Q: 运行程序窗口闪退

- 检查 `sim_data/` 目录是否在 `program.exe` 所在目录的上级（项目根目录）
- 运行 `program.exe` 时工作目录必须在 `AIO_Firmware_PIO/`

### Q: 自动测试超时

- 某些测试步骤较多（如 LH&LXW 有 20 步），可能需要更长时间
- 默认每步超时 20 帧，可在 `test_cases[]` 中调整

### Q: 编译输出 "Up to date"

PlatformIO 检测到代码未变更，无需重新编译。如需强制重编译，先删除 `.pio/build/AIO_native_sim/` 目录。

### Q: 网络功能不可用

仿真框架使用 WinSock2 实现真实 TCP/IP 网络，HTTP 请求、FTP 等均可正常使用。需确保 Windows 防火墙允许 `program.exe` 访问网络。