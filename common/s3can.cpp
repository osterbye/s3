#include "s3can.h"
#include "s3cellular.h"
//#include <errno.h>

S3Can::S3Can(Serial *debug, S3Cellular *cellular) :
    p_debug(debug),
    p_cellular(cellular)
{
    m_can = new CAN(CANRD, CANTD);
    m_standby = new DigitalOut(CANS);
}

S3Can::~S3Can()
{
}

void S3Can::init()
{
    p_debug->printf("Init CAN");
    m_standby->write(0);
    m_can->attach(callback(this, &S3Can::read));
}

void S3Can::write(const char *data)
{
    CANMessage msg(0x333, data, static_cast<char>(strlen(data)));
    m_can->write(msg);
}

void S3Can::read()
{
    CANMessage canMsg;
    m_can->read(canMsg);
    const uint16_t msgSize = canMsg.len + 7;
    char cellMsg[15];
    cellMsg[0] = (msgSize>>8)&0xFF; // msg size
    cellMsg[1] = msgSize&0xFF;      // msg size
    cellMsg[2] = S3Cellular::S3_MSG_CAN;
    cellMsg[3] = (canMsg.id>>24)&0xFF;
    cellMsg[4] = (canMsg.id>>16)&0xFF;
    cellMsg[5] = (canMsg.id>>8)&0xFF;
    cellMsg[6] = canMsg.id&0xFF;
    for (int i = 0; i < canMsg.len && i < 15; ++i) {
        cellMsg[7+i] = canMsg.data[i];
    }
    p_cellular->send(cellMsg, msgSize);
}
