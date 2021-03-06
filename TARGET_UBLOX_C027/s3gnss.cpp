#include "s3gnss.h"
#include "s3cellular.h"

#define NMEA183_MAX_MSG_LENGTH    (82*4)

S3Gnss::S3Gnss(Serial *debug, S3Cellular *cellular) :
    p_debug(debug),
    p_cellular(cellular),
    m_abort(false),
    m_refreshTime(250),
    m_doSetTime(true),
    m_date(0)
{
}

S3Gnss::~S3Gnss()
{
}

void S3Gnss::stopThread()
{
    m_abort = true;
}

void S3Gnss::loop()
{
    int ret;
    char buf[NMEA183_MAX_MSG_LENGTH] = "";
    //char link[128] = "";

    m_waitTimer.start();
    while (!m_abort) {
        while ((ret = m_gnss.getMessage(buf, sizeof(buf))) > 0) {
            int len = LENGTH(ret);
            //p_debug->printf("NMEA: %.*s\r\n", len-2, buf);
            if ((PROTOCOL(ret) == GnssParser::NMEA) && (len > 6)) {
                // talker is $GA=Galileo $GB=Beidou $GL=Glonass $GN=Combined $GP=GPS
                if ((buf[0] == '$') || buf[1] == 'G') {
                    #define _CHECK_TALKER(s) ((buf[3] == s[0]) && (buf[4] == s[1]) && (buf[5] == s[2]))
                    if (_CHECK_TALKER("GLL")) {
                        /*double la = 0, lo = 0;
                        floatBytes lat;
                        floatBytes lon;
                        char ch;
                        double time;
                        floatBytes t;
                        if (m_gnss.getNmeaAngle(1,buf,len,la) &&
                            m_gnss.getNmeaAngle(3,buf,len,lo) &&
                            m_gnss.getNmeaItem(5,buf,len,time) &&
                            m_gnss.getNmeaItem(6,buf,len,ch) && ch == 'A') {
                            lat.f = static_cast<float>(la);
                            lon.f = static_cast<float>(lo);
                            t.f = static_cast<float>(time);
                            const uint16_t msgSize = 19;
                            char msg[msgSize];
                            msg[0] = (msgSize>>8)&0xFF; // msg size
                            msg[1] = msgSize&0xFF;      // msg size
                            msg[2] = S3Cellular::S3_MSG_GNSS;
                            msg[3] = (m_date>>24)&0xFF;
                            msg[4] = (m_date>>16)&0xFF;
                            msg[5] = (m_date>>8)&0xFF;
                            msg[6] = m_date&0xFF;
                            msg[7] = t.b[3];
                            msg[8] = t.b[2];
                            msg[9] = t.b[1];
                            msg[10] = t.b[0];
                            msg[11] = lat.b[3];
                            msg[12] = lat.b[2];
                            msg[13] = lat.b[1];
                            msg[14] = lat.b[0];
                            msg[15] = lon.b[3];
                            msg[16] = lon.b[2];
                            msg[17] = lon.b[1];
                            msg[18] = lon.b[0];
                            p_cellular->send(msg, msgSize);
                            //p_debug->printf("GLL Location: %.5f %.5f\r\n", la, lo);
                        }*/

                        /*double la = 0, lo = 0;
                        char ch;
                        if (m_gnss.getNmeaAngle(1,buf,len,la) &&
                            m_gnss.getNmeaAngle(3,buf,len,lo) &&
                            m_gnss.getNmeaItem(6,buf,len,ch) && ch == 'A') {
                            p_debug->printf("GPS Location: %.5f %.5f\r\n", la, lo);
                            //sprintf(link, "I am here!\n"
                                          //"https://maps.google.com/?q=%.5f,%.5f", la, lo);
                        }*/
                    } else if (_CHECK_TALKER("GGA")) { // || _CHECK_TALKER("GNS") ) {
                        //double a = 0;
                        //if (m_gnss.getNmeaItem(9,buf,len,a)) // altitude msl [m]
                            //p_debug->printf("GPS Altitude: %.1f\r\n", a);

                        double la = 0, lo = 0;
                        floatBytes lat;
                        floatBytes lon;
                        char qual;
                        double time;
                        floatBytes t;
                        if (m_gnss.getNmeaAngle(2,buf,len,la) &&
                            m_gnss.getNmeaAngle(4,buf,len,lo) &&
                            m_gnss.getNmeaItem(1,buf,len,time) &&
                            m_gnss.getNmeaItem(6,buf,len,qual) && atoi(&qual) > 0) {
                            lat.f = static_cast<float>(la);
                            lon.f = static_cast<float>(lo);
                            t.f = static_cast<float>(time);
                            const uint16_t msgSize = 19;
                            char msg[msgSize];
                            msg[0] = (msgSize>>8)&0xFF; // msg size
                            msg[1] = msgSize&0xFF;      // msg size
                            msg[2] = S3Cellular::S3_MSG_GNSS;
                            msg[3] = (m_date>>24)&0xFF;
                            msg[4] = (m_date>>16)&0xFF;
                            msg[5] = (m_date>>8)&0xFF;
                            msg[6] = m_date&0xFF;
                            msg[7] = t.b[3];
                            msg[8] = t.b[2];
                            msg[9] = t.b[1];
                            msg[10] = t.b[0];
                            msg[11] = lat.b[3];
                            msg[12] = lat.b[2];
                            msg[13] = lat.b[1];
                            msg[14] = lat.b[0];
                            msg[15] = lon.b[3];
                            msg[16] = lon.b[2];
                            msg[17] = lon.b[1];
                            msg[18] = lon.b[0];
                            p_cellular->send(msg, msgSize);
                            //p_debug->printf("GGA Location: %.5f %.5f\r\n", la, lo);
                        }
                    } else if (_CHECK_TALKER("VTG")) {
                        /*double s = 0;
                        if (m_gnss.getNmeaItem(7,buf,len,s)) // speed [km/h]
                            p_debug->printf("GPS Speed: %.1f\r\n", s);*/
                    } else if (_CHECK_TALKER("RMC")) {
                        if (m_doSetTime){
                            int tod = 0;
                            int date = 0;
                            if (m_gnss.getNmeaItem(1,buf,len,tod, 10)) {// Time of Day
                                struct tm t = {0};
                                t.tm_hour = tod/10000;
                                t.tm_min = (tod/100) - (t.tm_hour*100);
                                t.tm_sec = tod - (t.tm_hour*10000) - (t.tm_min*100);
                                if (m_gnss.getNmeaItem(9,buf,len,date, 10)) {
                                    m_date = date;
                                    t.tm_mday = date/10000;
                                    t.tm_mon = (date/100) - (t.tm_mday*100) - 1;
                                    t.tm_year = (date - (t.tm_mday*10000) - ((t.tm_mon+1)*100)) + 100;
                                    set_time(mktime(&t));
                                    m_doSetTime = false;
                                }
                            }
                        }
                    }
                }
            }
        }

        int delay = m_refreshTime - m_waitTimer.read_ms();
        if (delay > 0) {
            Thread::wait(static_cast<uint32_t>(delay));
        }
        //Thread::wait((m_refreshTime - m_waitTimer.read_ms()));
        m_waitTimer.reset();
    }
}
