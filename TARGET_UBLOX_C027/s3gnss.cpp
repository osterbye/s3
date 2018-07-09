#include "s3gnss.h"

#define NMEA183_MAX_MSG_LENGTH    82

S3Gnss::S3Gnss(Serial *pc) : abort(false), pc(pc), refresh_time(1000), do_set_time(true) {
}

S3Gnss::~S3Gnss() {
}

void S3Gnss::stop_thread() {
    abort = true;
}

void S3Gnss::loop() {
    int ret;
    char buf[NMEA183_MAX_MSG_LENGTH] = "";
    //char link[128] = "";

    wait_timer.start();
    while (!abort) {
        while ((ret = gnss.getMessage(buf, sizeof(buf))) > 0) {
            int len = LENGTH(ret);
            //pc->printf("NMEA: %.*s\r\n", len-2, buf);
            if ((PROTOCOL(ret) == GnssParser::NMEA) && (len > 6)) {
                // talker is $GA=Galileo $GB=Beidou $GL=Glonass $GN=Combined $GP=GPS
                if ((buf[0] == '$') || buf[1] == 'G') {
                    #define _CHECK_TALKER(s) ((buf[3] == s[0]) && (buf[4] == s[1]) && (buf[5] == s[2]))
                    if (_CHECK_TALKER("GLL")) {
                        double la = 0, lo = 0;
                        char ch;
                        if (gnss.getNmeaAngle(1,buf,len,la) &&
                            gnss.getNmeaAngle(3,buf,len,lo) &&
                            gnss.getNmeaItem(6,buf,len,ch) && ch == 'A') {
                            pc->printf("GPS Location: %.5f %.5f\r\n", la, lo);
                            /*sprintf(link, "I am here!\n"
                                          "https://maps.google.com/?q=%.5f,%.5f", la, lo);*/
                        }
                    } else if (_CHECK_TALKER("GGA") || _CHECK_TALKER("GNS") ) {
                        double a = 0;
                        if (gnss.getNmeaItem(9,buf,len,a)) // altitude msl [m]
                            pc->printf("GPS Altitude: %.1f\r\n", a);
                    } else if (_CHECK_TALKER("VTG")) {
                        double s = 0;
                        if (gnss.getNmeaItem(7,buf,len,s)) // speed [km/h]
                            pc->printf("GPS Speed: %.1f\r\n", s);
                    } else if (_CHECK_TALKER("RMC")) {
                        if (do_set_time){
                            int tod = 0;
                            int date = 0;
                            if (gnss.getNmeaItem(1,buf,len,tod, 10)) {// Time of Day
                                struct tm t = {0};
                                t.tm_hour = tod/10000;
                                t.tm_min = (tod/100) - (t.tm_hour*100);
                                t.tm_sec = tod - (t.tm_hour*10000) - (t.tm_min*100);
                                if (gnss.getNmeaItem(9,buf,len,date, 10)) {
                                    t.tm_mday = date/10000;
                                    t.tm_mon = (date/100) - (t.tm_mday*100) - 1;
                                    t.tm_year = (date - (t.tm_mday*10000) - ((t.tm_mon+1)*100)) + 100;
                                    set_time(mktime(&t));
                                    do_set_time = false;
                                }
                            }
                        }
                    }
                }
            }
        }
        Thread::wait((refresh_time - wait_timer.read_ms()));
        wait_timer.reset();
    }
}
