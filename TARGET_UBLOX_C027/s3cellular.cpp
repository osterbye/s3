#include "s3cellular.h"

#define PIN             "0000"
#define APN             NULL
#define USERNAME        NULL
#define PASSWORD        NULL


#define MAX_MSG_SIZE    365

S3Cellular::S3Cellular(Serial *debug, S3Messages *messages) :
    p_debug(debug),
    p_messages(messages),
    m_state(STATE_DISCONNECTED),
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

void S3Cellular::stopThread()
{
    m_abort = true;
    // Trigger event so we can loop to thread abort
    p_messages->messageAvailable.set(NEW_SHORT_MESSAG_AVAILABLE | NEW_LONG_MESSAGE_AVAILABLE);
}

void S3Cellular::loop()
{
    m_waitTimer.start();
    uint32_t delay = 0;
    while (!m_abort) {
        if (STATE_CONNECTED == m_state) {
            p_messages->messageAvailable.wait_any(NEW_SHORT_MESSAG_AVAILABLE | NEW_LONG_MESSAGE_AVAILABLE);
            while (!p_messages->emptyLong()) {
                longMessage *msg = p_messages->getNextLong();
                if (msg != NULL && send(msg->data, msg->len)) {
                    //DBG_CEL("freeLong\n");
                    p_messages->freeLong(msg);
                } else {
                    break;
                }
            }
            while (!p_messages->emptyShort()) {
                shortMessage *msg = p_messages->getNextShort();
                if (msg != NULL && send(msg->data, msg->len)) {
                    //DBG_CEL("freeShort\n");
                    p_messages->freeShort(msg);
                } else {
                    break;
                }
            }
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
            uint64_t imei = static_cast<uint64_t>(atoll(m_interface->imei()));
            if (0 == imei) {
                DBG_CEL("Could not get IMEI.\n");
                m_state = STATE_INIT_CONNECT;
                delay = 1000;
            } else {
                p_messages->setDeviceId(imei);
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

bool S3Cellular::send(const void *data, nsapi_size_t size)
{
    //DBG_CEL("SEND\n");
    if (STATE_CONNECTED == m_state) {
        if (m_sockTcp->send(data, size) < 0) {
            //DBG_CEL("----------------------------------------------\n");
            DBG_CEL("FAILED TO SEND MESSAGE.\n");
            //DBG_CEL("----------------------------------------------\n");
            disconnect();
            connect();
            return false;
        }
        return true;
    }
    return false;
}

void S3Cellular::connectionStatus(nsapi_error_t error)
{
    DBG_CEL("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    DBG_CEL("Connection error: %d.\n", error);
    DBG_CEL("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    disconnect();
    connect();
}
