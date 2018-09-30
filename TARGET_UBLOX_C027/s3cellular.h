#ifndef S3CELLULAR_H_
#define S3CELLULAR_H_
#include "s3messages.h"
#include "UbloxATCellularInterface.h"
#include <mbed.h>

#define DEBUG_S3CELLULAR
#ifdef DEBUG_S3CELLULAR
#define DBG_CEL(...)          p_debug->printf("[S3Cellular]");p_debug->printf(__VA_ARGS__);p_debug->printf("\r\n")
#else
#define DBG_CEL(...)
#endif

class S3Cellular
{
public:
    S3Cellular(Serial *debug, S3Messages *messages);
    ~S3Cellular();

    void connect();
    void disconnect();
    bool isConnected();
    void stopThread();
    void loop();

private:
    bool send(const void *data, nsapi_size_t size);
    void connectionStatus(nsapi_error_t error);

private:
    enum cell_state_t{
        STATE_DISCONNECTED,
        STATE_INIT_CONNECT,
        STATE_INIT_IMEI,
        STATE_INIT_SERVER,
        STATE_INIT_OPEN_SOCKET,
        STATE_INIT_CONNECT_SOCKET,
        STATE_CONNECTED
    };

    Serial *p_debug;
    S3Messages *p_messages;
    UbloxATCellularInterface *m_interface;
    TCPSocket *m_sockTcp;
    SocketAddress *m_tcpServer;
    cell_state_t m_state;
    bool m_abort;
    Timer m_waitTimer;
    int m_refreshTime;
    PlatformMutex m_mutex;
};

#endif // S3CELLULAR_H_
