#include "s3imu.h"
#include "s3cellular.h"
#include "s3datatypes.h"
#include <sys/time.h>
//#include <errno.h>
//#include "benchmarks/benchmark_thread.h"

#define MAX_MSG_SIZE        421
//#define MAX_MSG_SIZE        32
#define ACCEL_PAYLOAD_SIZE  16

const char accelName[] = "ACCEL";
const uint8_t m_accelDelay[7] = {0, 100, 20, 8, 4, 2, 1};

S3Imu::S3Imu(PinName sda, PinName scl, uint8_t xgAddr, uint8_t mAddr, Serial *debug, S3Cellular *cellular) :
    p_debug(debug),
    p_cellular(cellular),
    m_threadAccel(NULL),
    m_accelAbort(false),
    m_accelAddress(xgAddr),
    m_gyroAbort(false),
    m_magAbort(false)
    //m_abort(false),
    //m_refreshTime(250)
{
    m_imu = new LSM9DS1(sda, scl, xgAddr, mAddr, debug);
}

int S3Imu::init()
{
    p_debug->printf("Starting IMU: ");
    int whoami = m_imu->begin();
    //p_debug->printf("%s\n", (err ? "Fail" : "OK"));
    p_debug->printf("%x\n", whoami);
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
            //m_threadAccel = new Thread(osPriorityNormal, 2128, NULL, accelName);
            m_threadAccel = new Thread(osPriorityNormal, 3072, NULL, accelName);
            m_threadAccel->start(callback(this, &S3Imu::aLoop));
        }
    }

    /*switch (aRate) {
    case XL_POWER_DOWN:
        if (m_threadAccel != NULL) {
            m_accelAbort = true;
            m_threadAccel->join();
        }
        break;
    case XL_ODR_10:
        if ()
        break;
    }*/
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



/*void S3Imu::stopThread() {
    m_abort = true;
}

void S3Imu::loop() {
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
    //return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
    //return static_cast<uint32_t>(currentTime.tv_sec * 1000000 + currentTime.tv_usec);
    //return static_cast<uint32_t>((currentTime.tv_sec*1000) + (currentTime.tv_usec/1000));
    return (int64_t)((int64_t)currentTime.tv_sec*1000) + ((int64_t)currentTime.tv_usec/1000);
}

void S3Imu::aLoop()
{
    //uint32_t testCounter = 0;
    //float aX = 0.0;
    //float aY = 0.0;
    //float aZ = 0.0;
    floatBytes aX;
    floatBytes aY;
    floatBytes aZ;
    char accelMsg[MAX_MSG_SIZE];
    uint16_t accelIndex = 0;
    time_t previousTime = 0;
    uint32_t milliseconds = 0;

    bool firstLoop = true;
    int32_t delay = 0;
    Timer waitTimer;
    waitTimer.start();
    while (!m_accelAbort) {
        if (0 == accelIndex) {
            accelMsg[accelIndex]    = 0x00; // msg size
            accelMsg[++accelIndex]  = 0x05; // msg size
            accelMsg[++accelIndex]  = S3Cellular::S3_MSG_ACCEL;
            accelMsg[++accelIndex]  = m_accelAddress;
            accelMsg[++accelIndex]  = ((m_accelOdr << 4) & 0xF0) | (m_accelScale & 0x0F);
        }

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

        //int64_t timenow = timestamp();
        //struct timeval currentTime;
        //gettimeofday(&currentTime, NULL);
        //return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
        //return static_cast<uint32_t>(currentTime.tv_sec * 1000000 + currentTime.tv_usec);
        //return static_cast<uint32_t>((currentTime.tv_sec*1000) + (currentTime.tv_usec/1000));
        //return ((int64_t)currentTime.tv_sec*1000) + ((int64_t)currentTime.tv_usec/1000);

        //uint32_t ts = time(NULL) * 1000;
        time_t seconds = time(NULL);
        if (previousTime != seconds) {
            previousTime = seconds;
            milliseconds = 0;
        } else {
            milliseconds = (milliseconds+m_accelDelay[m_accelOdr]) > 1000 ? 999 : (milliseconds+m_accelDelay[m_accelOdr]);
        }
        //p_debug->printf("Time: %s\r\n", ctime(&seconds));
        //int64_t milliseconds = static_cast<int64_t>(seconds * 1000);


        //if (aZ.f > 0.8)
            //p_debug->printf("timestamp:\r\n\t%lld\r\n\t%lld\r\n", timenow, seconds);

        accelMsg[++accelIndex] = (seconds>>24)&0xFF;
        accelMsg[++accelIndex] = (seconds>>16)&0xFF;
        accelMsg[++accelIndex] = (seconds>>8)&0xFF;
        accelMsg[++accelIndex] = seconds&0xFF;
        accelMsg[++accelIndex] = (milliseconds>>8)&0xFF;
        accelMsg[++accelIndex] = milliseconds&0xFF;
        accelMsg[++accelIndex] = aX.b[3];
        accelMsg[++accelIndex] = aX.b[2];
        accelMsg[++accelIndex] = aX.b[1];
        accelMsg[++accelIndex] = aX.b[0];
        accelMsg[++accelIndex] = aY.b[3];
        accelMsg[++accelIndex] = aY.b[2];
        accelMsg[++accelIndex] = aY.b[1];
        accelMsg[++accelIndex] = aY.b[0];
        accelMsg[++accelIndex] = aZ.b[3];
        accelMsg[++accelIndex] = aZ.b[2];
        accelMsg[++accelIndex] = aZ.b[1];
        accelMsg[++accelIndex] = aZ.b[0];

        if ((accelIndex + ACCEL_PAYLOAD_SIZE + 1) > MAX_MSG_SIZE || m_accelAbort) {
            ++accelIndex;
            accelMsg[0] = (accelIndex>>8)&0xFF; // msg size
            accelMsg[1] = accelIndex&0xFF;      // msg size
            p_cellular->send(accelMsg, accelIndex);
            accelIndex = 0;
            //p_debug->printf("X\r\n");
        }

        //print_thread_stack(m_threadAccel, p_debug);

        delay = m_accelDelay[m_accelOdr] - waitTimer.read_ms();
        if (delay > 0 && !m_accelAbort) {
            //p_debug->printf("%d\r\n", delay);
            Thread::wait(static_cast<uint32_t>(delay));
        }
        waitTimer.reset();
    }
}
