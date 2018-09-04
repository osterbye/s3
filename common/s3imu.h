#ifndef S3IMU_H_
#define S3IMU_H_
#include <mbed.h>
#include "LSM9DS1.h"

class LSM9DS1;
class S3Cellular;

class S3Imu
{
public:
    S3Imu(PinName sda, PinName scl, uint8_t xgAddr, uint8_t mAddr, Serial *debug, S3Cellular *cellular);

    int init();
    void accelConfiguration(accel_scale aScl, accel_odr aRate);
    void gyroConfiguration(gyro_scale gScl, gyro_odr gRate);
    void magConfiguration(mag_scale mScl, mag_odr mRate);
    //void stopThread();
    //void loop();
    //void calibrate();
    LSM9DS1* getIMU();

private:
    uint32_t timestamp();
    void aLoop();
    void gLoop();
    void mLoop();


private:
    Serial *p_debug;
    S3Cellular *p_cellular;
    LSM9DS1 *m_imu;

    Thread *m_threadAccel;
    //uint8_t m_accelDelay;
    bool m_accelAbort;
    uint8_t m_accelAddress;
    accel_odr m_accelOdr;
    accel_scale m_accelScale;


    Thread *m_threadGyro;
    uint8_t m_gyroDelay;
    bool m_gyroAbort;

    Thread *m_threadMag;
    uint8_t m_magDelay;
    bool m_magAbort;

    //bool m_abort;
    //Timer m_waitTimer;
    //int m_refreshTime;
};

#endif // S3IMU_H_
