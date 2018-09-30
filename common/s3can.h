#ifndef S3CAN_H_
#define S3CAN_H_
#include <mbed.h>

#define OBD_II_SUPPORT

class S3Messages;

class S3Can
{
public:
    S3Can(Serial *debug, S3Messages *messages);
    ~S3Can();

    void init();
    void write(CANMessage msg);

private:
    void read();

private:
#ifdef OBD_II_SUPPORT
    enum FRAME_CONTROL {
        SEND_REMAINING_FRAMES = 0,
        SEPARATION_TIME_NONE = 0,
        FLOW_CTS = 0,
        FLOW_WAIT,
        FLOW_ABORT,
        FRAME_SINGLE = 0x00,
        FRAME_FIRST = 0x10,
        FRAME_CONSECUTIVE = 0x20,
        FRAME_FLOW_CTRL = 0x30
    };
#endif

    Serial *p_debug;
    S3Messages *p_messages;

    CAN *m_can;
    DigitalOut *m_standby;
};

#endif // S3CAN_H_
