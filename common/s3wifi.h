#ifndef S3WIFI_H_
#define S3WIFI_H_
#include <mbed.h>
#include "ESP8266Interface.h"

#define DEBUG_S3WIFI
#ifdef DEBUG_S3WIFI
#define DBG_WIFI(...)          p_debug->printf("[S3Wifi] ");p_debug->printf(__VA_ARGS__);p_debug->printf("\r\n")
#else
#define DBG_WIFI(...)
#endif

class S3Wifi
{
public:
    enum cell_message_type {
        S3_MSG_GNSS,
        S3_MSG_CAN
    };

    S3Wifi(Serial *debug);
    ~S3Wifi();

    void connect();
    void disconnect();
    bool isConnected();
    bool send(const void *data, nsapi_size_t size, uint64_t imei);
    void stopThread();
    void loop();

    void connectionStatus(nsapi_event_t event, intptr_t reason);

private:
    enum wifi_state_t{
        STATE_DISCONNECTED,
        STATE_INIT_CONNECT,
        STATE_INIT_SERVER,
        STATE_INIT_OPEN_SOCKET,
        STATE_INIT_CONNECT_SOCKET,
        STATE_CONNECTED
    };

    Serial *p_debug;
    ESP8266Interface *m_interface;
    TCPSocket *m_sockTcp;
    SocketAddress *m_tcpServer;
    wifi_state_t m_state;
    bool m_abort;
    Timer m_waitTimer;
    int m_refreshTime;
    DigitalOut *m_enable;
};

#endif // S3WIFI_H_
