#include "s3wifi.h"

#define SSID        "osterbye"
#define PASSWORD    "fadbadfed4"

S3Wifi::S3Wifi(Serial *debug) :
    p_debug(debug),
    m_state(STATE_DISCONNECTED),
    m_abort(false),
    m_refreshTime(1000)
{
    DBG_WIFI("Starting wifi interface");
    m_enable = new DigitalOut(D2, 0);
    m_interface = new ESP8266Interface();
    m_interface->attach(callback(this, &S3Wifi::connectionStatus));
}

S3Wifi::~S3Wifi()
{
    disconnect();
    stopThread();
}

void S3Wifi::connect()
{
    m_sockTcp = new TCPSocket();
    m_tcpServer = new SocketAddress();
    m_state = STATE_INIT_CONNECT;
}

void S3Wifi::disconnect()
{
    m_sockTcp->close();
    m_interface->disconnect();
    delete m_sockTcp;
    delete m_tcpServer;
    m_state = STATE_DISCONNECTED;
}

bool S3Wifi::isConnected()
{
    return (STATE_CONNECTED == m_state);
}

bool S3Wifi::send(const void *data, nsapi_size_t size, uint64_t imei)
{
    if (STATE_CONNECTED == m_state) {
        nsapi_size_t len = size + 11;
        nsapi_size_or_error_t ret;

        if (len > 512)
            return false;

        //char header[11];
        const char *buf = (const char *) data;
        char msg[512];
        //uint64_t imei = static_cast<uint64_t>(atoll(m_interface->imei()));
        //DBG_WIFI("IMEI: %s\n", m_interface->imei());

        msg[0] = 's';
        msg[1] = '3';
        msg[2] = 0x01;
        msg[3] = (imei>>56)&0xFF;
        msg[4] = (imei>>48)&0xFF;
        msg[5] = (imei>>40)&0xFF;
        msg[6] = (imei>>32)&0xFF;
        msg[7] = (imei>>24)&0xFF;
        msg[8] = (imei>>16)&0xFF;
        msg[9] = (imei>>8)&0xFF;
        msg[10] = imei&0xFF;

        for (nsapi_size_t i = 0; i < size; ++i) {
            msg[11+i] = buf[i];
        }

        if ((ret = m_sockTcp->send(msg, len)) <= 0) {
            DBG_WIFI("----------------------------------------------\n");
            DBG_WIFI("FAILED TO SEND MESSAGE.\n");
            DBG_WIFI("----------------------------------------------\n");
            disconnect();
            connect();
            return false;
        }
        return true;
    }
    return false;
}

void S3Wifi::stopThread()
{
    m_abort = true;
}

void S3Wifi::loop()
{
    m_waitTimer.start();
    int delay = 0;
    int connectionRetry = 0;
    while (!m_abort) {

        if (STATE_CONNECTED == m_state) {

            delay = m_refreshTime - m_waitTimer.read_ms();
        } else if (STATE_INIT_CONNECT == m_state) {
            m_enable->write(1);
            int ret = 0;
            if ((ret = m_interface->connect(SSID, PASSWORD, NSAPI_SECURITY_WPA_WPA2)) != 0) {
                if (++connectionRetry < 10) {
                    m_enable->write(0);
                    DBG_WIFI("Could not connect. ret = %d", ret);
                    delay = 10;
                    DBG_WIFI("mac: %s", m_interface->get_mac_address());
                } else {
                    DBG_WIFI("Could not connect. Give up.");
                    m_state = STATE_DISCONNECTED;
                    connectionRetry = 0;
                    delay = 1000;
                }
            } else {
                DBG_WIFI("Interface is connected");
                m_state = STATE_INIT_SERVER;
                connectionRetry = 0;
                delay = 0;
            }
        } else if (STATE_INIT_SERVER == m_state) {
            if (m_interface->gethostbyname("ec2-52-17-166-122.eu-west-1.compute.amazonaws.com", m_tcpServer) != 0) {
                DBG_WIFI("Failed to look up host address.\n");
                delay = 10;
            } else {
                DBG_WIFI("Found host address: %s.\n", m_tcpServer->get_ip_address());
                m_tcpServer->set_port(3333);
                m_state = STATE_INIT_OPEN_SOCKET;
                delay = 0;
            }
        } else if (STATE_INIT_OPEN_SOCKET == m_state) {
            if (m_sockTcp->open(m_interface) != 0) {
                DBG_WIFI("Failed to open socket.\n");
                delay = 10;
            } else {
                DBG_WIFI("Socket is opened");
                m_sockTcp->set_timeout(10000);
                m_state = STATE_INIT_CONNECT_SOCKET;
                delay = 0;
            }
        } else if (STATE_INIT_CONNECT_SOCKET == m_state) {
            if (m_sockTcp->connect(*m_tcpServer) != 0) {
                DBG_WIFI("Failed to connect to socket %d on server %s.\n", m_tcpServer->get_port(), m_tcpServer->get_ip_address());
                disconnect();
                Thread::wait(1000);
                connect();
                //m_state = STATE_INIT_OPEN_SOCKET;
                delay = 1000;
            } else {
                DBG_WIFI("Socket is connected");
                m_state = STATE_CONNECTED;
                delay = 0;
            }
        } else { // STATE_DISCONNECTED
            delay = 1000;
        }

        if (delay > 0) {
            Thread::wait(static_cast<uint32_t>(delay));
        }
        m_waitTimer.reset();
    }
}

void S3Wifi::connectionStatus(nsapi_event_t event, intptr_t reason)
{
    DBG_WIFI("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    DBG_WIFI("Connection status: %d.\n", event);
    DBG_WIFI("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    //disconnect();
    //connect();
}
