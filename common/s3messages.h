#ifndef S3MESSAGES_H_
#define S3MESSAGES_H_
#include <mbed.h>

#define MESSAGE_HEADER_SIZE         11

#define MAX_SHORT_MSG_SIZE          32
typedef struct {
    nsapi_size_t len;
    uint8_t data[MAX_SHORT_MSG_SIZE];
} shortMessage;

#define MAX_LONG_MSG_SIZE           365
typedef struct {
    nsapi_size_t len;
    uint8_t data[MAX_LONG_MSG_SIZE];
} longMessage;

#define NEW_SHORT_MESSAG_AVAILABLE  0x01
#define NEW_LONG_MESSAGE_AVAILABLE  0x02

class S3Messages
{
public:
    enum cell_message_type {
        S3_MSG_GNSS,
        S3_MSG_CAN_OLD,
        S3_MSG_ACCEL,
        S3_MSG_CAN,
        S3_MSG_DEVICE
    };

    S3Messages();
    //~S3Messages();
    void setDeviceId(uint64_t &id);
    uint64_t getDeviceId();

    //bool allocShort(shortMessage *msg);
    //bool allocLong(longMessage *msg);
    shortMessage* allocShort();
    longMessage* allocLong();
    osStatus putShort(shortMessage *msg);
    osStatus putLong(longMessage *msg);
    //bool getShort(shortMessage *msg);
    //bool getLong(longMessage *msg);
    shortMessage* getNextShort();
    longMessage* getNextLong();
    bool emptyShort();
    bool emptyLong();
    osStatus freeShort(shortMessage *msg);
    osStatus freeLong(longMessage *msg);

    EventFlags messageAvailable;

private:
    Mail<shortMessage, 8> m_shortMailBox;
    Mail<longMessage, 3> m_longMailBox;
    uint64_t m_deviceId;


    /*Mail<shortMessage, 8> ShortMessageMailBox;
    Mail<longMessage, 3> LongMessageMailBox;*/
};

#endif // S3MESSAGES_H_
