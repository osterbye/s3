#include "mbed.h"
#include "s3can.h"
#include "s3cellular.h"
#include "s3gnss.h"
#include "s3sd.h"
//#include "spicar_mdm.h"
//#include "spicar_imu.h"
//#include "dispatcher.h"
//#include "console.h"

//#include "benchmarks/benchmark_thread.h"

DigitalOut led1(LED1);
Serial m_debugTerminal(USBTX, USBRX);
Timer m_waitTimer;

//Thread dispatcherThread;

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
    //SpiCar_MDM mdm(&pc);
    //Thread mdm_thread(osPriorityBelowNormal, 268*8*2);
    //SpiCar_IMU imu(SDA, SCL, LSM9DS1_PRIMARY_XG_ADDR, LSM9DS1_PRIMARY_M_ADDR, &pc);
    //Thread imu_thread(osPriorityBelowNormal, 96*8*2);

    err = gnssThread.start(callback(&gnss, &S3Gnss::loop));
    if (err) {
        // TODO
    }

    m_debugTerminal.printf("Initializing filesystem\r\n");
    sd.init();

    sd.listContent();

    /*if (mdm.initialize()) {
        mdm_thread.start(&mdm, &SpiCar_MDM::loop);
    }

    if (imu.initialize()) {
        imu_thread.start(&imu, &SpiCar_IMU::loop);
    }*/

    //console_init(&pc);
    //dispatcherThread.start(dispatcher_task);

    //char testStr[128];
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

        //console_task();

        // Benchmarks
        //print_thread_data(&Thread, &pc);
        //print_thread_data(&gnss_thread, &pc);
        //print_thread_data(&mdm_thread, &pc);
        //print_thread_data(&imu_thread, &pc);

        int delay = loopTime - m_waitTimer.read_ms();
        if (delay > 0) {
            Thread::wait(static_cast<uint32_t>(delay));
        }
        //Thread::wait((loopTime - m_waitTimer.read_ms()));
        m_waitTimer.reset();
        //wait(0.8);
    }    
}
