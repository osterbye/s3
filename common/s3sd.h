#ifndef S3SD_H_
#define S3SD_H_
#include <mbed.h>
//#include <Serial.h>

class SDBlockDevice;
//class Serial;
class FATFileSystem;

class S3Sd
{
public:
    S3Sd(Serial *debug);
    ~S3Sd();

    int init();
    void writeToLog(const char *data);

    void listContent();

    //void stop_thread();
    //void loop();

private:
    //GnssSerial gnss;
    //bool abort;
    Serial *p_debug;
    //Timer wait_timer;
    //int refresh_time;
    //bool do_set_time;
    SDBlockDevice *m_blockDevice;
    FATFileSystem *m_fileSystem;
};

#endif // S3SD_H_
