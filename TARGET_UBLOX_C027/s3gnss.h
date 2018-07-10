#ifndef S3GNSS_H_
#define S3GNSS_H_
//#include "mbed.h"
#include "gnss/gnss.h"

class S3Gnss
{
public:
    S3Gnss(Serial *pc);
    ~S3Gnss();

    void stop_thread();
    void loop();

private:
    GnssSerial gnss;
    bool abort;
    Serial *pc;
    Timer wait_timer;
    int refresh_time;
    bool do_set_time;
};

#endif // S3GNSS_H_
