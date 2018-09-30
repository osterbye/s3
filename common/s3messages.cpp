#include "s3messages.h"

#define CURRENT_MESSAGE_PROTOCOL    0x01

S3Messages::S3Messages() :
    m_deviceId(0)
{

}

/*S3Messages::~S3Messages()
{
}*/

void S3Messages::setDeviceId(uint64_t &id)
{
    m_deviceId = id;
}

uint64_t S3Messages::getDeviceId()
{
    return m_deviceId;
}

/*bool S3Messages::allocShort(shortMessage *msg)
{
    bool ret = false;
    msg = m_shortMailBox.alloc();
    if ((ret = (NULL != msg))) {
        msg->data[0] = 's';
        msg->data[1] = '3';
        msg->data[2] = CURRENT_MESSAGE_PROTOCOL;
        msg->data[3] = (m_deviceId>>56)&0xFF;
        msg->data[4] = (m_deviceId>>48)&0xFF;
        msg->data[5] = (m_deviceId>>40)&0xFF;
        msg->data[6] = (m_deviceId>>32)&0xFF;
        msg->data[7] = (m_deviceId>>24)&0xFF;
        msg->data[8] = (m_deviceId>>16)&0xFF;
        msg->data[9] = (m_deviceId>>8)&0xFF;
        msg->data[10] = m_deviceId&0xFF;
        msg->len = 11;
    }
    return ret;
}

bool S3Messages::allocLong(longMessage *msg)
{
    bool ret = false;
    msg = m_longMailBox.alloc();
    if ((ret = (NULL != msg))) {
        msg->data[0] = 's';
        msg->data[1] = '3';
        msg->data[2] = CURRENT_MESSAGE_PROTOCOL;
        msg->data[3] = (m_deviceId>>56)&0xFF;
        msg->data[4] = (m_deviceId>>48)&0xFF;
        msg->data[5] = (m_deviceId>>40)&0xFF;
        msg->data[6] = (m_deviceId>>32)&0xFF;
        msg->data[7] = (m_deviceId>>24)&0xFF;
        msg->data[8] = (m_deviceId>>16)&0xFF;
        msg->data[9] = (m_deviceId>>8)&0xFF;
        msg->data[10] = m_deviceId&0xFF;
        msg->len = 11;
    }
    return ret;
}*/

shortMessage* S3Messages::allocShort()
{
    shortMessage *msg = m_shortMailBox.alloc();
    if (NULL != msg) {
        msg->data[0] = 's';
        msg->data[1] = '3';
        msg->data[2] = CURRENT_MESSAGE_PROTOCOL;
        msg->data[3] = (m_deviceId>>56)&0xFF;
        msg->data[4] = (m_deviceId>>48)&0xFF;
        msg->data[5] = (m_deviceId>>40)&0xFF;
        msg->data[6] = (m_deviceId>>32)&0xFF;
        msg->data[7] = (m_deviceId>>24)&0xFF;
        msg->data[8] = (m_deviceId>>16)&0xFF;
        msg->data[9] = (m_deviceId>>8)&0xFF;
        msg->data[10] = m_deviceId&0xFF;
        msg->len = 11;
    }
    return msg;
}

longMessage* S3Messages::allocLong()
{
    longMessage* msg = m_longMailBox.alloc();
    if (NULL != msg) {
        msg->data[0] = 's';
        msg->data[1] = '3';
        msg->data[2] = CURRENT_MESSAGE_PROTOCOL;
        msg->data[3] = (m_deviceId>>56)&0xFF;
        msg->data[4] = (m_deviceId>>48)&0xFF;
        msg->data[5] = (m_deviceId>>40)&0xFF;
        msg->data[6] = (m_deviceId>>32)&0xFF;
        msg->data[7] = (m_deviceId>>24)&0xFF;
        msg->data[8] = (m_deviceId>>16)&0xFF;
        msg->data[9] = (m_deviceId>>8)&0xFF;
        msg->data[10] = m_deviceId&0xFF;
        msg->len = 11;
    }
    return msg;
}

osStatus S3Messages::putShort(shortMessage *msg)
{
    osStatus ret = m_shortMailBox.put(msg);
    if (ret >= 0)
        messageAvailable.set(NEW_SHORT_MESSAG_AVAILABLE);

    return ret;
}

osStatus S3Messages::putLong(longMessage *msg)
{
    osStatus ret = m_longMailBox.put(msg);
    if (ret >= 0)
        messageAvailable.set(NEW_LONG_MESSAGE_AVAILABLE);

    return ret;
}

/*bool S3Messages::getShort(shortMessage *msg)
{
    osEvent evt = m_shortMailBox.get();
    if (evt.status == osEventMail) {
        msg = static_cast<shortMessage*>(evt.value.p);
        return true;
    }
    return false;
}*/

/*bool S3Messages::getLong(longMessage *msg)
{
    osEvent evt = m_longMailBox.get();
    if (evt.status == osEventMail) {
        msg = static_cast<longMessage*>(evt.value.p);
        return true;
    }
    return false;
}*/

shortMessage* S3Messages::getNextShort()
{
    shortMessage *msg = NULL;
    osEvent evt = m_shortMailBox.get();
    if (evt.status == osEventMail) {
        msg = static_cast<shortMessage*>(evt.value.p);
    }
    return msg;
}

longMessage* S3Messages::getNextLong()
{
    longMessage *msg = NULL;
    osEvent evt = m_longMailBox.get();
    if (evt.status == osEventMail) {
        msg = static_cast<longMessage*>(evt.value.p);
    }
    return msg;
}

bool S3Messages::emptyShort()
{
    return m_shortMailBox.empty();
}

bool S3Messages::emptyLong()
{
    return m_longMailBox.empty();
}

osStatus S3Messages::freeShort(shortMessage *msg)
{
    return m_shortMailBox.free(msg);
}

osStatus S3Messages::freeLong(longMessage *msg)
{
    return m_longMailBox.free(msg);
}
