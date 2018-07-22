#ifndef S3GNSS_H_
#define S3GNSS_H_
#include "gnss/gnss.h"

union floatBytes{
  float f;
  char b[sizeof(float)];
};

union doubleBytes{
  double d;
  char b[sizeof(double)];
};

class S3Cellular;

class S3Gnss
{
public:
    S3Gnss(Serial *debug, S3Cellular *cellular);
    ~S3Gnss();

    void stopThread();
    void loop();

private:
    Serial *p_debug;
    S3Cellular *p_cellular;
    GnssSerial m_gnss;
    bool m_abort;
    Timer m_waitTimer;
    int m_refreshTime;
    bool m_doSetTime;
    int m_date;
};

#endif // S3GNSS_H_
