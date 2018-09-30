#include "mbed.h"
#include "s3can.h"
#include "s3cellular.h"
#include "s3device.h"
#include "s3gnss.h"
#include "s3imu.h"
#include "s3messages.h"
//#include "s3sd.h"
#include "s3version.h"

#include "mbed_stats.h"

DigitalOut led1(LED1);
Serial m_debugTerminal(USBTX, USBRX);
Timer m_waitTimer;

const char gnssName[] = "GNSS";
const char cellularName[] = "3G";

int main() {
    Thread::wait(3000);

    const int loopTime = 1000;
    bool abort = false;
    bool firstConnection = true;

    m_debugTerminal.printf("\r\n\r\nStarting S3 application\r\n");

    m_debugTerminal.printf("Creating Messages class\r\n");
    S3Messages messages;
    m_debugTerminal.printf("Creating Cellular class\r\n");
    S3Cellular cellular(&m_debugTerminal, &messages);
    m_debugTerminal.printf("Creating CAN class\r\n");
    S3Can can(&m_debugTerminal, &messages);
    m_debugTerminal.printf("Creating GNSS class\r\n");
    S3Gnss gnss(&m_debugTerminal, &messages);
    //m_debugTerminal.printf("Creating SD class\r\n");
    //S3Sd sd(&m_debugTerminal);
    m_debugTerminal.printf("Creating IMU class\r\n");
    S3Imu imu(SDA, SCL, 0xD6, 0x3C, &m_debugTerminal, &messages);
    bool imuIsRunning = false;

    Thread cellularThread(osPriorityBelowNormal, 1400, NULL, cellularName);
    osStatus err = cellularThread.start(callback(&cellular, &S3Cellular::loop));
    if (err) {
        // TODO
    } else {
        cellular.connect();
    }

    can.init();

    Thread gnssThread(osPriorityBelowNormal, 2160, NULL, gnssName);
    err = gnssThread.start(callback(&gnss, &S3Gnss::loop));
    if (err) {
        // TODO
    }

    /*m_debugTerminal.printf("Initializing filesystem\r\n");
    if (!sd.init()) {
        sd.listContent();
    }*/

    mbed_stats_heap_t heap_stats;

    int timeIterator = 5;

    m_waitTimer.start();
    while(!abort) {
        led1 = !led1;
        if (++timeIterator >= 5) {
            time_t seconds = time(NULL);
            m_debugTerminal.printf("Time: %s\r\n", ctime(&seconds));
            timeIterator = 0;

            mbed_stats_heap_get(&heap_stats);
            printf("Current heap: %lu\r\n", heap_stats.current_size);
            printf("Max heap size: %lu\r\n", heap_stats.max_size);
            printf("Reserved heap size: %lu\r\n", heap_stats.reserved_size);

            size_t cnt = osThreadGetCount();
            mbed_stats_stack_t *stats = (mbed_stats_stack_t*) malloc(cnt * sizeof(mbed_stats_stack_t));

            cnt = mbed_stats_stack_get_each(stats, cnt);
            for (size_t i = 0; i < cnt; i++) {
                printf("Thread: 0x%lX, Stack size: %lu, Max stack: %lu\r\n", stats[i].thread_id, stats[i].reserved_size, stats[i].max_size);
            }

            if (stats)
                free(stats);
        }

        if (firstConnection && cellular.isConnected()) {
            const uint8_t deviceLen = strlen(deviceType);
            const uint16_t payloadSize = deviceLen + 7;
            shortMessage *msg = messages.allocShort();
            if (NULL != msg) {
                msg->data[msg->len++] = (payloadSize>>8)&0xFF;
                msg->data[msg->len++] = payloadSize&0xFF;
                msg->data[msg->len++] = S3Messages::S3_MSG_DEVICE;
                msg->data[msg->len++] = FIRMWARE_VERSION_MAJOR;
                msg->data[msg->len++] = FIRMWARE_VERSION_MINOR;
                msg->data[msg->len++] = FIRMWARE_VERSION_REVISION;
                msg->data[msg->len++] = deviceLen&0xFF;
                memcpy(&msg->data[msg->len], deviceType, deviceLen);
                msg->len += deviceLen;
                messages.putShort(msg);
                messages.messageAvailable.set(NEW_SHORT_MESSAG_AVAILABLE);
                firstConnection = false;
            }
        }

        if (!imuIsRunning && cellular.isConnected()) {
            if (0 != imu.init()) {
                imu.accelConfiguration(A_SCALE_2G, XL_ODR_10);
                //imu.accelConfiguration(A_SCALE_16G, XL_ODR_952);
                imuIsRunning = true;
            }
        }

        /*char data[3] = {0x02, 0x01, 0x0D};
        CANMessage msg(0x7DF, data, 3);
        can.write(msg);*/

        int delay = loopTime - m_waitTimer.read_ms();
        if (delay > 0) {
            Thread::wait(static_cast<uint32_t>(delay));
        }
        m_waitTimer.reset();
    }    
}
