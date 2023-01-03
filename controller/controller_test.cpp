#include "controller.h"
#include "../IMU/imu.h"
#include "../DShot/dshot.h"
#include <chrono>

#define IMU_ADDRESS 0x6B

using namespace std;

int main()
{
    // Initialize data structures
    controller control("proto.cfg");    // Controller class, stores PID loop for each controller
    state_t state;                      // Stores information of drone state
    motor_cmd_t motor_cmd;              // Stores motor RPM commands

    // Initialize communication
    imu_init(IMU_ADDRESS);              // Initializes IMU
    // dshot_init();                       // Initializes DShot

    auto cur = chrono::high_resolution_clock::now();
    auto start = chrono::high_resolution_clock::now();

    while (true)
    {
        state.imu_data = imu_read_data();   // Read IMU data
        start = chrono::high_resolution_clock::now();
        motor_cmd = control.control_loop(0,0,0,0,state);    // Set motor commands
        cur = chrono::high_resolution_clock::now();
        cout << chrono::duration_cast<chrono::microseconds>(cur - start).count() << endl;
        // cout << motor_cmd.motor_1 << "    " << motor_cmd.motor_2 << "    " << motor_cmd.motor_3 << "    " << motor_cmd.motor_4 << "        ";
        usleep(10000);
        // cout << "\r" << flush;
    }

    return 0;
}
