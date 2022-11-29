#include "app_controller.h"
#include "common.h"
#include "interface.h"
#include "Arduino.h"

#define APP_CTRL_CONFIG_PATH "/sys.cfg"
#define MPU_CONFIG_PATH "/mpu.cfg"
#define RGB_CONFIG_PATH "/rgb.cfg"

void AppController::read_config(SysUtilConfig *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(APP_CTRL_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->power_mode = 0;           // 功耗模式（0为节能模式 1为性能模式）
        cfg->backLight = 80;           // 屏幕亮度（1-100）
        cfg->rotation = 4;             // 屏幕旋转方向
        cfg->auto_calibration_mpu = 1; // 是否自动校准陀螺仪 0关闭自动校准 1打开自动校准
        cfg->mpu_order = 0;            // 操作方向
        cfg->auto_start_app = "None";  // 无指定开机自启APP
        this->write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[12] = {0};
        analyseParam(info, 12, param);
        cfg->ssid_0 = param[0];
        cfg->password_0 = param[1];
        cfg->ssid_1 = param[2];
        cfg->password_1 = param[3];
        cfg->ssid_2 = param[4];
        cfg->password_2 = param[5];
        cfg->power_mode = atol(param[6]);
        cfg->backLight = atol(param[7]);
        cfg->rotation = atol(param[8]);
        cfg->auto_calibration_mpu = atol(param[9]);
        cfg->mpu_order = atol(param[10]);
        cfg->auto_start_app = param[11]; // 开机自启APP的name
    }
}

void AppController::write_config(SysUtilConfig *cfg)
{
    char tmp[25];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    w_data = w_data + cfg->ssid_0 + "\n";
    w_data = w_data + cfg->password_0 + "\n";
    w_data = w_data + cfg->ssid_1 + "\n";
    w_data = w_data + cfg->password_1 + "\n";
    w_data = w_data + cfg->ssid_2 + "\n";
    w_data = w_data + cfg->password_2 + "\n";
    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->power_mode);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->backLight);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->rotation);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->auto_calibration_mpu);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->mpu_order);
    w_data += tmp;

    w_data = w_data + cfg->auto_start_app + "\n";

    g_flashCfg.writeFile(APP_CTRL_CONFIG_PATH, w_data.c_str());

    // 立即生效相关配置
    screen.setBackLight(cfg->backLight / 100.0);
    tft->setRotation(cfg->rotation);
    mpu.setOrder(cfg->mpu_order);
}

void AppController::read_config(SysMpuConfig *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(MPU_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->x_gyro_offset = 0;
        cfg->y_gyro_offset = 0;
        cfg->z_gyro_offset = 0;
        cfg->x_accel_offset = 0;
        cfg->y_accel_offset = 0;
        cfg->z_accel_offset = 0;

        this->write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[6] = {0};
        analyseParam(info, 6, param);
        cfg->x_gyro_offset = atol(param[0]);
        cfg->y_gyro_offset = atol(param[1]);
        cfg->z_gyro_offset = atol(param[2]);
        cfg->x_accel_offset = atol(param[3]);
        cfg->y_accel_offset = atol(param[4]);
        cfg->z_accel_offset = atol(param[5]);
    }
}

void AppController::write_config(SysMpuConfig *cfg)
{
    char tmp[25];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->x_gyro_offset);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->y_gyro_offset);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->z_gyro_offset);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->x_accel_offset);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->y_accel_offset);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->z_accel_offset);
    w_data += tmp;
    g_flashCfg.writeFile(MPU_CONFIG_PATH, w_data.c_str());
}

void AppController::read_config(RgbConfig *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(RGB_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->mode = 1;
        cfg->min_value_0 = 1;
        cfg->min_value_1 = 32;
        cfg->min_value_2 = 255;
        cfg->max_value_0 = 255;
        cfg->max_value_1 = 255;
        cfg->max_value_2 = 255;
        cfg->step_0 = 1;
        cfg->step_1 = 1;
        cfg->step_2 = 1;
        cfg->min_brightness = 0.15;
        cfg->max_brightness = 0.25;
        cfg->brightness_step = 0.001;
        cfg->time = 30;

        this->write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[14] = {0};
        analyseParam(info, 14, param);
        cfg->mode = atol(param[0]);
        cfg->min_value_0 = atol(param[1]);
        cfg->min_value_1 = atol(param[2]);
        cfg->min_value_2 = atol(param[3]);
        cfg->max_value_0 = atol(param[4]);
        cfg->max_value_1 = atol(param[5]);
        cfg->max_value_2 = atol(param[6]);
        cfg->step_0 = atol(param[7]);
        cfg->step_1 = atol(param[8]);
        cfg->step_2 = atol(param[9]);
        cfg->min_brightness = atof(param[10]);
        cfg->max_brightness = atof(param[11]);
        cfg->brightness_step = atof(param[12]);
        cfg->time = atol(param[13]);
    }
}

void AppController::write_config(RgbConfig *cfg)
{
    char tmp[25];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->mode);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->min_value_0);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->min_value_1);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->min_value_2);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->max_value_0);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->max_value_1);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%u\n", cfg->max_value_2);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->step_0);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->step_1);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->step_2);
    w_data += tmp;

    if (cfg->min_brightness < 0.01)
    {
        // 限制
        cfg->min_brightness = 0.01;
    }
    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%f\n", cfg->min_brightness);
    w_data += tmp;

    if (cfg->max_brightness < 0.01)
    {
        // 限制
        cfg->max_brightness = 0.01;
    }
    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%f\n", cfg->max_brightness);
    w_data += tmp;

    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%f\n", cfg->brightness_step);
    w_data += tmp;

    if (cfg->time < 10)
    {
        cfg->time = 10;
    }
    memset(tmp, 0, 25);
    snprintf(tmp, 25, "%d\n", cfg->time);
    w_data += tmp;

    g_flashCfg.writeFile(RGB_CONFIG_PATH, w_data.c_str());

    // 初始化RGB灯 HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV,
                            rgb_cfg.min_value_0, rgb_cfg.min_value_1, rgb_cfg.min_value_2,
                            rgb_cfg.max_value_0, rgb_cfg.max_value_1, rgb_cfg.max_value_2,
                            rgb_cfg.step_0, rgb_cfg.step_1, rgb_cfg.step_2,
                            rgb_cfg.min_brightness, rgb_cfg.max_brightness,
                            rgb_cfg.brightness_step, rgb_cfg.time};
    // 初始化RGB任务
    set_rgb_and_run(&rgb_setting);
}

void AppController::deal_config(APP_MESSAGE_TYPE type,
                                const char *key, char *value)
{
    switch (type)
    {

    case APP_MESSAGE_GET_PARAM:
    {
        if (!strcmp(key, "ssid_0"))
        {
            snprintf(value, 32, "%s", sys_cfg.ssid_0.c_str());
        }
        else if (!strcmp(key, "password_0"))
        {
            snprintf(value, 32, "%s", sys_cfg.password_0.c_str());
        }
        else if (!strcmp(key, "ssid_1"))
        {
            snprintf(value, 32, "%s", sys_cfg.ssid_1.c_str());
        }
        else if (!strcmp(key, "password_1"))
        {
            snprintf(value, 32, "%s", sys_cfg.password_1.c_str());
        }
        if (!strcmp(key, "ssid_2"))
        {
            snprintf(value, 32, "%s", sys_cfg.ssid_2.c_str());
        }
        else if (!strcmp(key, "password_2"))
        {
            snprintf(value, 32, "%s", sys_cfg.password_2.c_str());
        }
        else if (!strcmp(key, "power_mode"))
        {
            snprintf(value, 32, "%u", sys_cfg.power_mode);
        }
        else if (!strcmp(key, "backLight"))
        {
            snprintf(value, 32, "%u", sys_cfg.backLight);
        }
        else if (!strcmp(key, "rotation"))
        {
            snprintf(value, 32, "%u", sys_cfg.rotation);
        }
        else if (!strcmp(key, "auto_calibration_mpu"))
        {
            snprintf(value, 32, "%u", sys_cfg.auto_calibration_mpu);
        }
        else if (!strcmp(key, "mpu_order"))
        {
            snprintf(value, 32, "%u", sys_cfg.mpu_order);
        }
        else if (!strcmp(key, "min_brightness"))
        {
            snprintf(value, 32, "%f", rgb_cfg.min_brightness);
        }
        else if (!strcmp(key, "max_brightness"))
        {
            snprintf(value, 32, "%f", rgb_cfg.max_brightness);
        }
        else if (!strcmp(key, "time"))
        {
            snprintf(value, 32, "%u", rgb_cfg.time);
        }
        else if (!strcmp(key, "auto_start_app"))
        {
            snprintf(value, 32, "%s", sys_cfg.auto_start_app.c_str());
        }
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
        if (!strcmp(key, "ssid_0"))
        {
            sys_cfg.ssid_0 = value;
        }
        else if (!strcmp(key, "password_0"))
        {
            sys_cfg.password_0 = value;
        }
        else if (!strcmp(key, "ssid_1"))
        {
            sys_cfg.ssid_1 = value;
        }
        else if (!strcmp(key, "password_1"))
        {
            sys_cfg.password_1 = value;
        }
        else if (!strcmp(key, "ssid_2"))
        {
            sys_cfg.ssid_2 = value;
        }
        else if (!strcmp(key, "password_2"))
        {
            sys_cfg.password_2 = value;
        }
        else if (!strcmp(key, "power_mode"))
        {
            sys_cfg.power_mode = atol(value);
        }
        else if (!strcmp(key, "backLight"))
        {
            sys_cfg.backLight = atol(value);
        }
        else if (!strcmp(key, "rotation"))
        {
            sys_cfg.rotation = atol(value);
        }
        else if (!strcmp(key, "auto_calibration_mpu"))
        {
            sys_cfg.auto_calibration_mpu = atol(value);
            if (0 == sys_cfg.auto_calibration_mpu)
            {
                this->write_config(&this->mpu_cfg);
            }
        }
        else if (!strcmp(key, "mpu_order"))
        {
            sys_cfg.mpu_order = atol(value);
        }
        else if (!strcmp(key, "min_brightness"))
        {
            rgb_cfg.min_brightness = atof(value);
        }
        else if (!strcmp(key, "max_brightness"))
        {
            rgb_cfg.max_brightness = atof(value);
        }
        else if (!strcmp(key, "time"))
        {
            rgb_cfg.time = atol(value);
        }
        else if (!strcmp(key, "auto_start_app"))
        {
            sys_cfg.auto_start_app = value;
        }
    }
    break;
    case APP_MESSAGE_READ_CFG:
    {
        read_config(&sys_cfg);
        // read_config(&mpu_cfg);
        read_config(&rgb_cfg);
    }
    break;
    case APP_MESSAGE_WRITE_CFG:
    {
        write_config(&sys_cfg);
        // write_config(&mpu_cfg);  // 在取消自动校准的时候已经写过一次了
        write_config(&rgb_cfg);
    }
    break;
    default:
        break;
    }
}