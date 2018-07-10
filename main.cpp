#include "mbed.h"
#include "s3gnss.h"
//#include "spicar_mdm.h"
//#include "spicar_imu.h"
//#include "dispatcher.h"
//#include "console.h"

//#include "benchmarks/benchmark_thread.h"

DigitalOut led1(LED1);
Serial debugTerminal(USBTX, USBRX);
Timer waitTimer;

//Thread dispatcherThread;

int main() {
    const int loopTime = 1000;
    bool abort = false;
    S3Gnss gnss(&debugTerminal);

    Thread gnss_thread(osPriorityBelowNormal, 104*8*2);
    //SpiCar_MDM mdm(&pc);
    //Thread mdm_thread(osPriorityBelowNormal, 268*8*2);
    //SpiCar_IMU imu(SDA, SCL, LSM9DS1_PRIMARY_XG_ADDR, LSM9DS1_PRIMARY_M_ADDR, &pc);
    //Thread imu_thread(osPriorityBelowNormal, 96*8*2);

    osStatus err = gnss_thread.start(callback(&gnss, &S3Gnss::loop));
    if (err) {
        // TODO
    }

    /*if (mdm.initialize()) {
        mdm_thread.start(&mdm, &SpiCar_MDM::loop);
    }

    if (imu.initialize()) {
        imu_thread.start(&imu, &SpiCar_IMU::loop);
    }*/

    //console_init(&pc);
    //dispatcherThread.start(dispatcher_task);

    waitTimer.start();
    while(!abort) {
        led1 = !led1;
        time_t seconds = time(NULL);
        debugTerminal.printf("Time: %s\r\n", ctime(&seconds));

        //console_task();

        // Benchmarks
        //print_thread_data(&Thread, &pc);
        //print_thread_data(&gnss_thread, &pc);
        //print_thread_data(&mdm_thread, &pc);
        //print_thread_data(&imu_thread, &pc);

        Thread::wait((loopTime - waitTimer.read_ms()));
        waitTimer.reset();
        //wait(0.8);
    }    
}
