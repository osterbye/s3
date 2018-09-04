#include "s3cellular.h"

#define PIN         "0000"
#define APN         NULL
#define USERNAME    NULL
#define PASSWORD    NULL

S3Cellular::S3Cellular(Serial *debug) :
    p_debug(debug),
    m_state(STATE_DISCONNECTED),
    m_imei(0),
    m_abort(false),
    m_refreshTime(1000)
{
    DBG_CEL("Starting cellular interface");
    m_interface = new UbloxATCellularInterface();
    m_interface->connection_status_cb(callback(this, &S3Cellular::connectionStatus));
}

S3Cellular::~S3Cellular()
{
    disconnect();
    stopThread();
}

void S3Cellular::connect()
{
    m_interface->set_credentials(APN, USERNAME, PASSWORD);
    m_sockTcp = new TCPSocket();
    m_tcpServer = new SocketAddress();
    m_state = STATE_INIT_CONNECT;
}

void S3Cellular::disconnect()
{
    m_sockTcp->close();
    m_interface->disconnect();
    delete m_sockTcp;
    delete m_tcpServer;
    m_state = STATE_DISCONNECTED;
}

bool S3Cellular::isConnected()
{
    return (STATE_CONNECTED == m_state);
}

bool S3Cellular::send(const void *data, nsapi_size_t size)
{
    m_mutex.lock();
    if (STATE_CONNECTED == m_state) {
    // CHANGED FOR TESTING
    //if (true) {
        nsapi_size_t len = size + 11;
        nsapi_size_or_error_t ret;

        if (len > 512) {
            m_mutex.unlock();
            return false;
        }

        const char *buf = static_cast<const char *>(data);
        char header[512];
        //uint64_t imei = static_cast<uint64_t>(atoll(m_interface->imei()));
        //DBG_CEL("IMEI: %s\n", m_interface->imei());

        header[0] = 's';
        header[1] = '3';
        header[2] = 0x01;
        header[3] = (m_imei>>56)&0xFF;
        header[4] = (m_imei>>48)&0xFF;
        header[5] = (m_imei>>40)&0xFF;
        header[6] = (m_imei>>32)&0xFF;
        header[7] = (m_imei>>24)&0xFF;
        header[8] = (m_imei>>16)&0xFF;
        header[9] = (m_imei>>8)&0xFF;
        header[10] = m_imei&0xFF;

        for (nsapi_size_t i = 0; i < size; ++i) {
            header[11+i] = buf[i];
        }

        // COMMENTED OUT FOR TESTING
        if ((ret = m_sockTcp->send(header, len)) <= 0) {
            //DBG_CEL("----------------------------------------------\n");
            DBG_CEL("FAILED TO SEND MESSAGE.\n");
            //DBG_CEL("----------------------------------------------\n");
            disconnect();
            connect();
            m_mutex.unlock();
            return false;
        }
        m_mutex.unlock();
        return true;
    }
    m_mutex.unlock();
    return false;
}

void S3Cellular::stopThread()
{
    m_abort = true;
}

void S3Cellular::loop()
{
    m_waitTimer.start();
    uint32_t delay = 0;
    while (!m_abort) {
        if (STATE_CONNECTED == m_state) {
            delay = static_cast<uint32_t>(m_refreshTime - m_waitTimer.read_ms());
        } else if (STATE_INIT_CONNECT == m_state) {
            if (m_interface->connect(PIN) != 0) {
                DBG_CEL("Could not connect. Check antenna, APN, PIN, etc.\n");
                delay = 1000;
            } else {
                DBG_CEL("Interface is connected");
                m_state = STATE_INIT_IMEI;
                delay = 0;
            }
        } else if (STATE_INIT_IMEI == m_state) {
            m_imei = static_cast<uint64_t>(atoll(m_interface->imei()));
            if (0 == m_imei) {
                DBG_CEL("Could not get IMEI.\n");
                m_state = STATE_INIT_CONNECT;
                delay = 1000;
            } else {
                m_state = STATE_INIT_SERVER;
                delay = 0;
            }
        }else if (STATE_INIT_SERVER == m_state) {
            if (m_interface->gethostbyname("ec2-52-17-166-122.eu-west-1.compute.amazonaws.com", m_tcpServer) != 0) {
                DBG_CEL("Failed to look up host address.\n");
                delay = 1000;
            } else {
                DBG_CEL("Found host address: %s.\n", m_tcpServer->get_ip_address());
                m_tcpServer->set_port(3333);
                m_state = STATE_INIT_OPEN_SOCKET;
                delay = 0;
            }
        } else if (STATE_INIT_OPEN_SOCKET == m_state) {
            if (m_sockTcp->open(m_interface) != 0) {
                DBG_CEL("Failed to open socket.\n");
                delay = 1000;
            } else {
                DBG_CEL("Socket is opened");
                m_sockTcp->set_timeout(10000);
                m_state = STATE_INIT_CONNECT_SOCKET;
                delay = 0;
            }
        } else if (STATE_INIT_CONNECT_SOCKET == m_state) {
            if (m_sockTcp->connect(*m_tcpServer) != 0) {
                DBG_CEL("Failed to connect to socket %d on server %s.\n", m_tcpServer->get_port(), m_tcpServer->get_ip_address());
                disconnect();
                Thread::wait(1000);
                connect();
                delay = 1000;
            } else {
                DBG_CEL("Socket is connected");
                m_state = STATE_CONNECTED;
                delay = 0;
            }
        } else { // STATE_DISCONNECTED
            delay = 2000;
        }

        if (delay > 0) {
            Thread::wait(delay);
        }
        m_waitTimer.reset();
    }
}

void S3Cellular::connectionStatus(nsapi_error_t error)
{
    DBG_CEL("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    DBG_CEL("Connection error: %d.\n", error);
    DBG_CEL("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    disconnect();
    connect();
}
