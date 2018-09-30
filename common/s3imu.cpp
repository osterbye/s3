#include "s3imu.h"
#include "s3messages.h"
#include "s3datatypes.h"
#include <sys/time.h>

#define ACCEL_PAYLOAD_SIZE  18

const char accelName[] = "ACCEL";
const uint8_t m_accelDelay[7] = {0, 100, 20, 8, 4, 2, 1};

S3Imu::S3Imu(PinName sda, PinName scl, uint8_t xgAddr, uint8_t mAddr, Serial *debug, S3Messages *messages) :
    p_debug(debug),
    p_messages(messages),
    m_threadAccel(NULL),
    m_accelAbort(false),
    m_accelAddress(xgAddr),
    m_gyroAbort(false),
    m_magAbort(false)
{
    m_imu = new LSM9DS1(sda, scl, xgAddr, mAddr, debug);
}

int S3Imu::init()
{
    //p_debug->printf("Starting IMU: ");
    int whoami = m_imu->begin();
    //p_debug->printf("%s\n", (err ? "Fail" : "OK"));
    //p_debug->printf("%x\n", whoami);
    return whoami;
}

void S3Imu::accelConfiguration(accel_scale aScl, accel_odr aRate)
{
    if (aScl > 3 || aRate > 6)
        return;

    m_accelScale = aScl;
    bool resetThread = (aRate != m_accelOdr);
    m_accelOdr = aRate;

    m_imu->setAccelScale(aScl);
    m_imu->setAccelODR(aRate);

    if (XL_POWER_DOWN == aRate || resetThread) {
        if (NULL != m_threadAccel) {
            p_debug->printf("Destroying accel thread\n");
            m_accelAbort = true;
            m_threadAccel->join();
            delete m_threadAccel;
            m_threadAccel = NULL;
        }
    }
    if (XL_POWER_DOWN != aRate) {
        if (NULL == m_threadAccel) {
            p_debug->printf("Creating accel thread\n");
            m_accelAbort = false;
            m_threadAccel = new Thread(osPriorityNormal, 3072, NULL, accelName);
            m_threadAccel->start(callback(this, &S3Imu::aLoop));
        }
    }
}

void S3Imu::gyroConfiguration(gyro_scale gScl, gyro_odr gRate)
{
    m_imu->setGyroScale(gScl);
    m_imu->setGyroODR(gRate);
}

void S3Imu::magConfiguration(mag_scale mScl, mag_odr mRate)
{
    m_imu->setMagScale(mScl);
    m_imu->setMagODR(mRate);
}

/*void S3Imu::loop() {
    m_waitTimer.start();
    uint32_t delay = 0;
    while (!m_abort) {
        while (!m_imu->tempAvailable());
        m_imu->readTemp();
        while (!m_imu->magAvailable());
        m_imu->readMag();
        while (!m_imu->accelAvailable());
        m_imu->readAccel();
        while (!m_imu->gyroAvailable());
        m_imu->readGyro();

        delay = static_cast<uint32_t>(m_refreshTime - m_waitTimer.read_ms());
        if (delay > 0) {
            Thread::wait(delay);
        }
        m_waitTimer.reset();
    }
}*/

/*void S3Imu::calibrate()
{
    m_imu->calibrate(1);
    m_imu->calibrateMag(0);
}*/

LSM9DS1* S3Imu::getIMU()
{
    return m_imu;
}

int64_t S3Imu::timestamp()
{
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return (int64_t)((int64_t)currentTime.tv_sec*1000) + ((int64_t)currentTime.tv_usec/1000);
}

void S3Imu::aLoop()
{
    floatBytes aX;
    floatBytes aY;
    floatBytes aZ;
    longMessage *msg = NULL;
    time_t previousTime = 0;
    uint32_t milliseconds = 0;

    bool firstLoop = true;
    int32_t delay = 0;
    Timer waitTimer;
    waitTimer.start();
    while (!m_accelAbort) {
        if (NULL == msg) {
            //p_debug->printf("alloc long\r\n");
            msg = p_messages->allocLong();
            if (NULL != msg) {
                msg->data[msg->len++] = 0x00;
                msg->data[msg->len++] = 0x05;
                msg->data[msg->len++] = S3Messages::S3_MSG_ACCEL;
                msg->data[msg->len++] = m_accelAddress;
                msg->data[msg->len++] = ((m_accelOdr << 4) & 0xF0) | (m_accelScale & 0x0F);
            }
        }

        if (NULL != msg) {
            while (!m_imu->accelAvailable() && !m_accelAbort) {
                Thread::wait(1U);
            }

            if (firstLoop) {
                waitTimer.reset();
                firstLoop = false;
            }
            m_imu->readAccel();
            aX.f = m_imu->calcAccel(m_imu->ax);
            aY.f = m_imu->calcAccel(m_imu->ay);
            aZ.f = m_imu->calcAccel(m_imu->az);
            //p_debug->printf("X: %.4f\tY: %.4f\tZ: %.4f\r\n", aX, aY, aZ);

            time_t seconds = time(NULL);
            if (previousTime != seconds) {
                previousTime = seconds;
                milliseconds = 0;
            } else {
                milliseconds = (milliseconds+m_accelDelay[m_accelOdr]) >= 1000 ? 999 : (milliseconds+m_accelDelay[m_accelOdr]);
            }
            //p_debug->printf("Time: %s\r\n", ctime(&seconds));

            msg->data[msg->len++] = (seconds>>24)&0xFF;
            msg->data[msg->len++] = (seconds>>16)&0xFF;
            msg->data[msg->len++] = (seconds>>8)&0xFF;
            msg->data[msg->len++] = seconds&0xFF;
            msg->data[msg->len++] = (milliseconds>>8)&0xFF;
            msg->data[msg->len++] = milliseconds&0xFF;
            msg->data[msg->len++] = aX.b[3];
            msg->data[msg->len++] = aX.b[2];
            msg->data[msg->len++] = aX.b[1];
            msg->data[msg->len++] = aX.b[0];
            msg->data[msg->len++] = aY.b[3];
            msg->data[msg->len++] = aY.b[2];
            msg->data[msg->len++] = aY.b[1];
            msg->data[msg->len++] = aY.b[0];
            msg->data[msg->len++] = aZ.b[3];
            msg->data[msg->len++] = aZ.b[2];
            msg->data[msg->len++] = aZ.b[1];
            msg->data[msg->len++] = aZ.b[0];

            if ((msg->len + ACCEL_PAYLOAD_SIZE) >= MAX_LONG_MSG_SIZE || m_accelAbort) {
                msg->data[MESSAGE_HEADER_SIZE]   = (msg->len>>8)&0xFF;  // msg size
                msg->data[MESSAGE_HEADER_SIZE+1] = msg->len&0xFF;       // msg size
                p_messages->putLong(msg);
                p_messages->messageAvailable.set(NEW_LONG_MESSAGE_AVAILABLE);
                msg = NULL;
                //p_debug->printf("X\r\n");
            }
        }

        delay = m_accelDelay[m_accelOdr] - waitTimer.read_ms();
        if (delay > 0 && !m_accelAbort) {
            //p_debug->printf("%d\r\n", delay);
            Thread::wait(static_cast<uint32_t>(delay));
        }
        waitTimer.reset();
    }
}
