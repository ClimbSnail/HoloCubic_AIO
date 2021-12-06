#include "imu.h"
#include "common.h"

const char *active_type_info[] = {"TURN_RIGHT", "RETURN",
                                  "TURN_LEFT", "UP",
                                  "DOWN", "GO_FORWORD",
                                  "SHAKE", "UNKNOWN"};

IMU::IMU()
{
    action_info.isValid = true;
    action_info.active = UNKNOWN;
    action_info.long_time = true;
}

void IMU::init()
{
    Wire.begin(IMU_I2C_SDA, IMU_I2C_SCL);
    Wire.setClock(400000);
    unsigned long timeout = 5000;
    unsigned long preMillis = millis();
    mpu = MPU6050(0x68);
    while (!mpu.testConnection() && !doDelayMillisTime(timeout, &preMillis, false))
        ;

    if (!mpu.testConnection())
    {
        Serial.print(F("Unable to connect to MPU6050.\n"));
        return;
    }

    Serial.print(F("Initialization MPU6050 now, Please don't move.\n"));
    mpu.initialize();

    if (g_cfg.auto_calibration_mpu == 0)
    {
        // supply your own gyro offsets here, scaled for min sensitivity
        mpu.setXGyroOffset(g_cfg.mpu_config.x_gyro_offset);
        mpu.setYGyroOffset(g_cfg.mpu_config.y_gyro_offset);
        mpu.setZGyroOffset(g_cfg.mpu_config.z_gyro_offset);
        mpu.setXAccelOffset(g_cfg.mpu_config.x_accel_offset);
        mpu.setYAccelOffset(g_cfg.mpu_config.y_accel_offset);
        mpu.setZAccelOffset(g_cfg.mpu_config.z_accel_offset); // 1688 factory default for my test chip
    }
    else
    {
        // 启动自动校准
        // 7次循环自动校正
        mpu.CalibrateAccel(7);
        mpu.CalibrateGyro(7);
        mpu.PrintActiveOffsets();

        g_cfg.mpu_config.x_gyro_offset = mpu.getXGyroOffset();
        g_cfg.mpu_config.y_gyro_offset = mpu.getYGyroOffset();
        g_cfg.mpu_config.z_gyro_offset = mpu.getZGyroOffset();
        g_cfg.mpu_config.x_accel_offset = mpu.getXAccelOffset();
        g_cfg.mpu_config.y_accel_offset = mpu.getYAccelOffset();
        g_cfg.mpu_config.z_accel_offset = mpu.getZAccelOffset();
        mpu_config_save(NULL, &g_cfg); // 保存mup校准数据
    }

    Serial.print(F("Initialization MPU6050 success.\n"));
}

Imu_Action *IMU::update(int interval)
{
    mpu.getMotion6(&(action_info.ax), &(action_info.ay), &(action_info.az),
                   &(action_info.gx), &(action_info.gy), &(action_info.gz));
    // 原先判断的只是加速度，现在要加上陀螺仪
    if (millis() - last_update_time > interval)
    {
        if (!action_info.isValid)
        {
            if (action_info.ay > 4000)
            {
                encoder_diff--;
                action_info.isValid = 1;
                action_info.active = TURN_LEFT;
            }
            else if (action_info.ay < -4000)
            {
                encoder_diff++;
                action_info.isValid = 1;
                action_info.active = TURN_RIGHT;
            }
            else if (action_info.ay > 1000 || action_info.ay < -1000)
            {
                // 震动检测
                encoder_diff--;
                action_info.isValid = 1;
                action_info.active = SHAKE;
            }
            else
            {
                action_info.isValid = 0;
            }
        }

        // if (!action_info.isValid)
        // {
        //     if (action_info.gy > 4000)
        //     {
        //         encoder_diff--;
        //         action_info.isValid = 1;
        //         action_info.active = TURN_LEFT;
        //     }
        //     else if (action_info.gy < -4000)
        //     {
        //         encoder_diff++;
        //         action_info.isValid = 1;
        //         action_info.active = TURN_RIGHT;
        //     }
        //     else
        //     {
        //         action_info.isValid = 0;
        //     }
        // }

        // 上下
        // if (!action_info.isValid)
        // {
        //     if (action_info.ax > 3000 && action_info.ax <= 5000)
        //     {
        //         action_info.isValid = 1;
        //         action_info.active = UP;
        //     }
        //     else if (action_info.ax < -3000 && action_info.ax >= -5000)
        //     {
        //         action_info.isValid = 1;
        //         action_info.active = DOWN;
        //     }
        //     else
        //     {
        //         action_info.isValid = 0;
        //     }
        // }

        if (!action_info.isValid)
        {
            if (action_info.ax > 5000)
            {
                action_info.isValid = 1;
                action_info.active = UP;
                delay(500);
                mpu.getMotion6(&(action_info.ax), &(action_info.ay), &(action_info.az),
                               &(action_info.gx), &(action_info.gy), &(action_info.gz));
                if (action_info.ax > 5000)
                {
                    action_info.isValid = 1;
                    action_info.active = GO_FORWORD;
                    encoder_state = LV_INDEV_STATE_PR;
                }
            }
            else if (action_info.ax < -5000)
            {
                action_info.isValid = 1;
                action_info.active = DOWN;
                delay(500);
                mpu.getMotion6(&(action_info.ax), &(action_info.ay), &(action_info.az),
                               &(action_info.gx), &(action_info.gy), &(action_info.gz));
                if (action_info.ax < -5000)
                {
                    action_info.isValid = 1;
                    action_info.active = RETURN;
                    encoder_state = LV_INDEV_STATE_REL;
                }
            }
            else if (action_info.ax > 1000 || action_info.ax < -1000)
            {
                // 震动检测
                action_info.isValid = 1;
                action_info.active = SHAKE;
            }
            else
            {
                action_info.isValid = 0;
            }
        }

        last_update_time = millis();

        // 操作方向进行调整
        if (UNKNOWN != action_info.active)
        {
            action_info.active = (ACTIVE_TYPE)((action_info.active + 0) % UNKNOWN);
        }
    }
    return &action_info;
}
