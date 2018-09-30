#ifndef S3GNSS_H_
#define S3GNSS_H_
#include "gnss/gnss.h"

class S3Messages;

class S3Gnss
{
public:
    S3Gnss(Serial *debug, S3Messages *messages);
    ~S3Gnss();

    void stopThread();
    void loop();

private:
    Serial *p_debug;
    S3Messages *p_messages;
    GnssSerial m_gnss;
    bool m_abort;
    Timer m_waitTimer;
    int m_refreshTime;
    bool m_doSetTime;
    int m_date;
};

#endif // S3GNSS_H_
