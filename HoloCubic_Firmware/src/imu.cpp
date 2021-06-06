#include "imu.h"

void IMU::init()
{
	Wire.begin(IMU_I2C_SDA, IMU_I2C_SCL);
	Wire.setClock(400000);
	while (!imu.testConnection())
		;
	imu.initialize();
	active_info.active = UNKNOWN;
}

Imu_Active* IMU::update(int interval)
{
	imu.getMotion6(&(active_info.ax), &(active_info.ay), &(active_info.az), 
				&(active_info.gx), &(active_info.gy), &(active_info.gz));

	if (millis() - last_update_time > interval)
	{
		if (active_info.ay > 3000 && !active_info.isValid)
		{
			encoder_diff--;
			active_info.isValid = 1;
			active_info.active = TURN_LEFT;
		}
		else if (active_info.ay < -3000)
		{
			encoder_diff++;
			active_info.isValid = 1;
			active_info.active = TURN_RIGHT;
		}
		else
		{
			active_info.isValid = 0;
		}

		if (active_info.ax > 5000 && !active_info.isValid)
		{
			delay(1000);
			imu.getMotion6(&(active_info.ax), &(active_info.ay), &(active_info.az), 
						&(active_info.gx), &(active_info.gy), &(active_info.gz));
			if (active_info.ax > 5000)
			{
				active_info.isValid = 1;
				active_info.active = GO_FORWORD;
				encoder_state = LV_INDEV_STATE_PR;
			}
		}
		else if (active_info.ax < -5000 && !active_info.isValid)
		{
			delay(1000);
			imu.getMotion6(&(active_info.ax), &(active_info.ay), &(active_info.az), 
						&(active_info.gx), &(active_info.gy), &(active_info.gz));
			if (active_info.ax < -5000)
			{
				active_info.isValid = 1;
				active_info.active = RETURN;
				encoder_state = LV_INDEV_STATE_REL;
			}
		}
		else
		{
			active_info.isValid = 0;
		}
		last_update_time = millis();
	}
	return &active_info;
}
