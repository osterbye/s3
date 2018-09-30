#include "s3can.h"
#include "s3cellular.h"

#define CAN_BAUD_RATE       500000

S3Can::S3Can(Serial *debug, S3Messages *messages) :
    p_debug(debug),
    p_messages(messages)
{
    m_can = new CAN(CANRD, CANTD, CAN_BAUD_RATE);
    m_standby = new DigitalOut(CANS);
}

S3Can::~S3Can()
{
}

void S3Can::init()
{
    p_debug->printf("Init CAN\n");
    m_can->mode(CAN::Normal);
    m_standby->write(0);
    m_can->attach(callback(this, &S3Can::read));
}

void S3Can::write(CANMessage msg)
{
    m_can->write(msg);
}

void S3Can::read()
{
    CANMessage canMsg;
    m_can->read(canMsg);
    const uint16_t payloadSize = canMsg.len + 11;

    if ((payloadSize + MESSAGE_HEADER_SIZE) <= MAX_SHORT_MSG_SIZE) {
        time_t seconds = time(NULL);
        shortMessage *msg = p_messages->allocShort();
        if (NULL != msg) {
            msg->data[msg->len++] = (payloadSize>>8)&0xFF;
            msg->data[msg->len++] = payloadSize&0xFF;
            msg->data[msg->len++] = S3Messages::S3_MSG_CAN;
            msg->data[msg->len++] = (seconds>>24)&0xFF;
            msg->data[msg->len++] = (seconds>>16)&0xFF;
            msg->data[msg->len++] = (seconds>>8)&0xFF;
            msg->data[msg->len++] = seconds&0xFF;
            msg->data[msg->len++] = (canMsg.id>>24)&0xFF;
            msg->data[msg->len++] = (canMsg.id>>16)&0xFF;
            msg->data[msg->len++] = (canMsg.id>>8)&0xFF;
            msg->data[msg->len++] = canMsg.id&0xFF;
            for (int i = 0; i < canMsg.len; ++i) {
                msg->data[msg->len++] = canMsg.data[i];
                p_debug->printf("0x%02x ", canMsg.data[i]);
            }
            p_messages->putShort(msg);
            p_messages->messageAvailable.set(NEW_SHORT_MESSAG_AVAILABLE);
        }
    } else {
        p_debug->printf("CAN message too long: %d bytes", (payloadSize + MESSAGE_HEADER_SIZE));
    }
#ifdef OBD_II_SUPPORT
    switch (canMsg.data[0]&0xF0) {
    case FRAME_FIRST:
    {
        char flowData[3] = {(FRAME_FLOW_CTRL|FLOW_CTS), SEND_REMAINING_FRAMES, SEPARATION_TIME_NONE};
        int flowID = static_cast<int>(canMsg.id - 8);
        CANMessage flowMsg(flowID, flowData, 3);
        m_can->write(flowMsg);
        break;
    }
    case FRAME_SINGLE:
    case FRAME_CONSECUTIVE:
    default:
        // Do nothing here, since we're not parsing messages runtime.
        break;
    }
#endif
}
