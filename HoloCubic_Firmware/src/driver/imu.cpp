#include "imu.h"
#include "common.h"

const char *active_type_info[] = {"TURN_RIGHT", "RETURN",
                                  "TURN_LEFT", "UP",
                                  "DOWN", "GO_FORWORD",
                                  "SHAKE", "UNKNOWN"};

IMU::IMU()
{
    action_info.isValid = false;
    action_info.active = ACTIVE_TYPE::UNKNOWN;
    action_info.long_time = true;
    // 初始化数据
    for (int pos = 0; pos < ACTION_HISTORY_BUF_LEN; ++pos)
    {
        // act_info_history.push_back(UNKNOWN);
        act_info_history[pos] = UNKNOWN;
    }
    act_info_history_ind = ACTION_HISTORY_BUF_LEN - 1;
    this->order = 0; // 表示方位
}

void IMU::init(uint8_t order, uint8_t auto_calibration,
               SysMpuConfig *mpu_cfg)
{
    this->setOrder(order); // 设置方向
    Wire.begin(IMU_I2C_SDA, IMU_I2C_SCL);
    Wire.setClock(400000);
    unsigned long timeout = 5000;
    unsigned long preMillis = millis();
    // mpu = MPU6050(0x68, &Wire);
    mpu = MPU6050(0x68);
    Serial.print(F("Unable to connect to 4.\n"));
    while (!mpu.testConnection() && !doDelayMillisTime(timeout, &preMillis, false))
        ;
    Serial.print(F("Unable to connect to 5.\n"));

    if (!mpu.testConnection())
    {
        Serial.print(F("Unable to connect to MPU6050.\n"));
        return;
    }

    Serial.print(F("Initialization MPU6050 now, Please don't move.\n"));
    mpu.initialize();

    if (auto_calibration == 0)
    {
        // supply your own gyro offsets here, scaled for min sensitivity
        mpu.setXGyroOffset(mpu_cfg->x_gyro_offset);
        mpu.setYGyroOffset(mpu_cfg->y_gyro_offset);
        mpu.setZGyroOffset(mpu_cfg->z_gyro_offset);
        mpu.setXAccelOffset(mpu_cfg->x_accel_offset);
        mpu.setYAccelOffset(mpu_cfg->y_accel_offset);
        mpu.setZAccelOffset(mpu_cfg->z_accel_offset); // 1688 factory default for my test chip
    }
    else
    {
        // 启动自动校准
        // 7次循环自动校正
        mpu.CalibrateAccel(7);
        mpu.CalibrateGyro(7);
        mpu.PrintActiveOffsets();

        mpu_cfg->x_gyro_offset = mpu.getXGyroOffset();
        mpu_cfg->y_gyro_offset = mpu.getYGyroOffset();
        mpu_cfg->z_gyro_offset = mpu.getZGyroOffset();
        mpu_cfg->x_accel_offset = mpu.getXAccelOffset();
        mpu_cfg->y_accel_offset = mpu.getYAccelOffset();
        mpu_cfg->z_accel_offset = mpu.getZAccelOffset();
    }

    Serial.print(F("Initialization MPU6050 success.\n"));
}

void IMU::setOrder(uint8_t order) // 设置方向
{
    this->order = order; // 表示方位
}

bool IMU::Encoder_GetIsPush(void)
{
#ifdef PEAK
    return (digitalRead(CONFIG_ENCODER_PUSH_PIN) == LOW);
#else
    return false;
#endif
}

ImuAction *IMU::update(int interval)
{
    getVirtureMotion6(&action_info);
    // 原先判断的只是加速度，现在要加上陀螺仪
    if (millis() - last_update_time > interval)
    {
        if (!action_info.isValid)
        {
            if (action_info.v_ay > 4000)
            {
                encoder_diff--;
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::TURN_LEFT;
            }
            else if (action_info.v_ay < -4000)
            {
                encoder_diff++;
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::TURN_RIGHT;
            }
            else if (action_info.v_ay > 1000 || action_info.v_ay < -1000)
            {
                // 震动检测
                encoder_diff--;
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::SHAKE;
            }
            else
            {
                action_info.isValid = 0;
            }
        }

        if (!action_info.isValid)
        {
            if (action_info.v_ax > 5000)
            {
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::UP;
                delay(500);
                getVirtureMotion6(&action_info);
                if (action_info.v_ax > 5000)
                {
                    action_info.isValid = 1;
                    action_info.active = ACTIVE_TYPE::GO_FORWORD;
                    encoder_state = LV_INDEV_STATE_PR;
                }
            }
            else if (action_info.v_ax < -5000)
            {
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::DOWN;
                delay(500);
                getVirtureMotion6(&action_info);
                if (action_info.v_ax < -5000)
                {
                    action_info.isValid = 1;
                    action_info.active = ACTIVE_TYPE::RETURN;
                    encoder_state = LV_INDEV_STATE_REL;
                }
            }
            else if (action_info.v_ax > 1000 || action_info.v_ax < -1000)
            {
                // 震动检测
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::SHAKE;
            }
            else
            {
                action_info.isValid = 0;
            }
        }

        last_update_time = millis();
    }
    return &action_info;
}

ImuAction *IMU::getAction(void)
{
    ImuAction tmp_info;
    getVirtureMotion6(&tmp_info);

    tmp_info.active = ACTIVE_TYPE::UNKNOWN;

    if (ACTIVE_TYPE::UNKNOWN == tmp_info.active)
    {
        if (tmp_info.v_ay > 4000)
        {
            tmp_info.active = ACTIVE_TYPE::TURN_LEFT;
        }
        else if (tmp_info.v_ay < -4000)
        {
            tmp_info.active = ACTIVE_TYPE::TURN_RIGHT;
        }
        else if (tmp_info.v_ay > 1000 || tmp_info.v_ay < -1000)
        {
            // 震动检测
            tmp_info.active = ACTIVE_TYPE::SHAKE;
        }
    }

    if (ACTIVE_TYPE::UNKNOWN == tmp_info.active)
    {
        if (tmp_info.v_ax > 5000)
        {
            tmp_info.active = ACTIVE_TYPE::UP;
        }
        else if (tmp_info.v_ax < -5000)
        {
            tmp_info.active = ACTIVE_TYPE::DOWN;
        }
        else if (action_info.v_ax > 1000 || action_info.v_ax < -1000)
        {
            // 震动检测
            tmp_info.active = ACTIVE_TYPE::SHAKE;
        }
    }

    act_info_history_ind = (act_info_history_ind + 1) % ACTION_HISTORY_BUF_LEN;
    int index = act_info_history_ind;
    act_info_history[index] = tmp_info.active;

    if (!action_info.isValid)
    {
        int second = (index + ACTION_HISTORY_BUF_LEN - 1) % ACTION_HISTORY_BUF_LEN;
        int third = (index + ACTION_HISTORY_BUF_LEN - 2) % ACTION_HISTORY_BUF_LEN;
        if (act_info_history[index] == act_info_history[second] && act_info_history[second] == act_info_history[third])
        {
            if (ACTIVE_TYPE::UP == tmp_info.active)
            {
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::GO_FORWORD;
            }
            else if (ACTIVE_TYPE::DOWN == tmp_info.active)
            {
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::RETURN;
            }
        }
        else if (ACTIVE_TYPE::UNKNOWN != tmp_info.active)
        {
            action_info.isValid = 1;
            action_info.active = tmp_info.active;
        }
    }

    return &action_info;
}

void IMU::getVirtureMotion6(ImuAction *action_info)
{
    mpu.getMotion6(&(action_info->v_ax), &(action_info->v_ay),
                   &(action_info->v_az), &(action_info->v_gx),
                   &(action_info->v_gy), &(action_info->v_gz));

    if (order & X_DIR_TYPE)
    {
        action_info->v_ax = -action_info->v_ax;
        action_info->v_gx = -action_info->v_gx;
    }

    if (order & Y_DIR_TYPE)
    {
        action_info->v_ay = -action_info->v_ay;
        action_info->v_gy = -action_info->v_gy;
    }

    if (order & Z_DIR_TYPE)
    {
        action_info->v_az = -action_info->v_az;
        action_info->v_gz = -action_info->v_gz;
    }

    if (order & XY_DIR_TYPE)
    {
        int16_t swap_tmp;
        swap_tmp = action_info->v_ax;
        action_info->v_ax = action_info->v_ay;
        action_info->v_ay = swap_tmp;
        swap_tmp = action_info->v_gx;
        action_info->v_gx = action_info->v_gy;
        action_info->v_gy = swap_tmp;
    }
}