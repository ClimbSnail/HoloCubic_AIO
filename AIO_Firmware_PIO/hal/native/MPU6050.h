#ifndef MPU6050_STUB_H
#define MPU6050_STUB_H

class MPU6050
{
public:
    void initialize() {}
    bool testConnection() { return true; }
    void setXGyroOffset(int16_t offset) {}
    void setYGyroOffset(int16_t offset) {}
    void setZGyroOffset(int16_t offset) {}
    void setXAccelOffset(int16_t offset) {}
    void setYAccelOffset(int16_t offset) {}
    void setZAccelOffset(int16_t offset) {}
    void getMotion6(int16_t *ax, int16_t *ay, int16_t *az, int16_t *gx, int16_t *gy, int16_t *gz) {}
};

#endif