#include "imu.h"

void IMU::init()
{
    Wire.begin(IMU_I2C_SDA, IMU_I2C_SCL);
    Wire.setClock(400000);
    while (!imu.testConnection())
        ;
    imu.initialize();
    action_info.active = UNKNOWN;
}

Imu_Action *IMU::update(int interval)
{
    imu.getMotion6(&(action_info.ax), &(action_info.ay), &(action_info.az),
                   &(action_info.gx), &(action_info.gy), &(action_info.gz));

    if (millis() - last_update_time > interval)
    {
        if (action_info.ay > 4000 && !action_info.isValid)
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
        else
        {
            action_info.isValid = 0;
        }

        if (action_info.ax > 5000 && !action_info.isValid)
        {
            delay(300);
            imu.getMotion6(&(action_info.ax), &(action_info.ay), &(action_info.az),
                           &(action_info.gx), &(action_info.gy), &(action_info.gz));
            if (action_info.ax > 5000)
            {
                action_info.isValid = 1;
                action_info.active = GO_FORWORD;
                encoder_state = LV_INDEV_STATE_PR;
            }
        }
        else if (action_info.ax < -5000 && !action_info.isValid)
        {
            delay(300);
            imu.getMotion6(&(action_info.ax), &(action_info.ay), &(action_info.az),
                           &(action_info.gx), &(action_info.gy), &(action_info.gz));
            if (action_info.ax < -5000)
            {
                action_info.isValid = 1;
                action_info.active = RETURN;
                encoder_state = LV_INDEV_STATE_REL;
            }
        }
        else
        {
            action_info.isValid = 0;
        }
        last_update_time = millis();
    }
    return &action_info;
}
