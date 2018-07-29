#include "mbed.h"
#include "s3can.h"
#include "s3cellular.h"
#include "s3gnss.h"
#include "s3sd.h"
//#include "spicar_imu.h"

//#include "benchmarks/benchmark_thread.h"

DigitalOut led1(LED1);
Serial m_debugTerminal(USBTX, USBRX);
Timer m_waitTimer;

int main() {
    const int loopTime = 1000;
    bool abort = false;

    m_debugTerminal.printf("Starting S3 application\r\n");

    m_debugTerminal.printf("Creating Cellular class\r\n");
    S3Cellular cellular(&m_debugTerminal);
    m_debugTerminal.printf("Creating CAN class\r\n");
    S3Can can(&m_debugTerminal, &cellular);
    m_debugTerminal.printf("Creating GNSS class\r\n");
    S3Gnss gnss(&m_debugTerminal, &cellular);
    m_debugTerminal.printf("Creating SD class\r\n");
    S3Sd sd(&m_debugTerminal);

    Thread cellularThread(osPriorityBelowNormal);
    osStatus err = cellularThread.start(callback(&cellular, &S3Cellular::loop));
    if (err) {
        // TODO
    } else {
        cellular.connect();
    }

    can.init();

    Thread gnssThread(osPriorityBelowNormal, 104*8*2);
    //SpiCar_IMU imu(SDA, SCL, LSM9DS1_PRIMARY_XG_ADDR, LSM9DS1_PRIMARY_M_ADDR, &pc);
    //Thread imu_thread(osPriorityBelowNormal, 96*8*2);

    err = gnssThread.start(callback(&gnss, &S3Gnss::loop));
    if (err) {
        // TODO
    }

    m_debugTerminal.printf("Initializing filesystem\r\n");
    if (!sd.init()) {
        sd.listContent();
    }


    /*if (imu.initialize()) {
        imu_thread.start(&imu, &SpiCar_IMU::loop);
    }*/

    m_waitTimer.start();
    while(!abort) {
        led1 = !led1;
        time_t seconds = time(NULL);
        m_debugTerminal.printf("Time: %s\r\n", ctime(&seconds));


        /*if (cellular.isConnected()) {
            snprintf(testStr, 128, "Device time: %s", ctime(&seconds));
            size_t len = strlen(testStr);
            cellular.send(testStr, len);
        }*/


        // Benchmarks
        //print_thread_data(&Thread, &pc);
        //print_thread_data(&gnss_thread, &pc);
        //print_thread_data(&mdm_thread, &pc);
        //print_thread_data(&imu_thread, &pc);

        int delay = loopTime - m_waitTimer.read_ms();
        if (delay > 0) {
            Thread::wait(static_cast<uint32_t>(delay));
        }
        m_waitTimer.reset();
    }    
}
