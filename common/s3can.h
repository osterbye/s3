#ifndef S3CAN_H_
#define S3CAN_H_
#include <mbed.h>

class S3Cellular;
class S3Wifi;

class S3Can
{
public:
    S3Can(Serial *debug, S3Cellular *cellular, S3Wifi *wifi);
    ~S3Can();

    void init();
    void write(const char *data);

private:
    void read();

private:
    Serial *p_debug;
    S3Cellular *p_cellular;
    S3Wifi *p_wifi;

    CAN *m_can;
    DigitalOut *m_standby;
};

#endif // S3CAN_H_
