#ifndef S3CELLULAR_H_
#define S3CELLULAR_H_
#include <mbed.h>
#include "UbloxATCellularInterface.h"

#define DEBUG_S3CELLULAR
#ifdef DEBUG_S3CELLULAR
#define DBG_CEL(...)          p_debug->printf("[S3Cellular]");p_debug->printf(__VA_ARGS__);p_debug->printf("\r\n")
#else
#define DBG_CEL(...)
#endif

class S3Cellular
{
public:
    enum cell_message_type {
        S3_MSG_GNSS,
        S3_MSG_CAN
    };

    S3Cellular(Serial *debug);
    ~S3Cellular();

    void connect();
    void disconnect();
    bool isConnected();
    bool send(const void *data, nsapi_size_t size);
    void stopThread();
    void loop();

    void connectionStatus(nsapi_error_t error);

private:
    enum cell_state_t{
        STATE_DISCONNECTED,
        STATE_INIT_CONNECT,
        STATE_INIT_SERVER,
        STATE_INIT_OPEN_SOCKET,
        STATE_INIT_CONNECT_SOCKET,
        STATE_CONNECTED
    };

    Serial *p_debug;
    UbloxATCellularInterface *m_interface;
    TCPSocket *m_sockTcp;
    SocketAddress *m_tcpServer;
    cell_state_t m_state;
    bool m_abort;
    Timer m_waitTimer;
    int m_refreshTime;
};

#endif // S3CELLULAR_H_
