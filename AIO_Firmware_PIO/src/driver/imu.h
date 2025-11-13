#ifndef IMU_H
#define IMU_H

#include <I2Cdev.h>
#include <MPU6050.h>
#include "lv_port_indev.h"
#include <list>
#define ACTION_HISTORY_BUF_LEN 5

extern int32_t encoder_diff;
extern lv_indev_state_t encoder_state;

extern const char *active_type_info[];

enum ACTIVE_TYPE
{
    TURN_RIGHT = 0,
    RETURN,
    TURN_LEFT,
    UP,
    DOWN,
    GO_FORWORD,
    SHAKE,
    UNKNOWN
};

// Direction type
enum MPU_DIR_TYPE
{
    NORMAL_DIR_TYPE = 0,
    X_DIR_TYPE = 0x01,
    Y_DIR_TYPE = 0x02,
    Z_DIR_TYPE = 0x04,
    XY_DIR_TYPE = 0x08
};

struct SysMpuConfig
{
    int16_t x_gyro_offset;
    int16_t y_gyro_offset;
    int16_t z_gyro_offset;

    int16_t x_accel_offset;
    int16_t y_accel_offset;
    int16_t z_accel_offset;
};

struct ImuAction
{
    volatile ACTIVE_TYPE active;
    boolean isValid;
    boolean long_time;
    int16_t v_ax; // v represents virtual parameters (used to adjust the initial orientation of the 6050)
    int16_t v_ay;
    int16_t v_az;
    int16_t v_gx;
    int16_t v_gy;
    int16_t v_gz;
};

class IMU
{
private:
    MPU6050 mpu;
    int flag;
    long last_update_time;
    uint8_t order; // Represents orientation, whether x and y are swapped

public:
    ImuAction action_info;
    // Used to store historical actions
    // std::list<ACTIVE_TYPE> act_info_history;
    ACTIVE_TYPE act_info_history[ACTION_HISTORY_BUF_LEN];
    int act_info_history_ind; // Flag indicating storage position

public:
    IMU();
    void init(uint8_t order, uint8_t auto_calibration,
              SysMpuConfig *mpu_cfg);
    void setOrder(uint8_t order); // Set direction
    bool Encoder_GetIsPush(void); // Adapter for Peak's encoder middle button for power on/off
    ImuAction *update(int interval);
    ImuAction *getAction(void); // Get action
    void getVirtureMotion6(ImuAction *action_info);
};

#endif
