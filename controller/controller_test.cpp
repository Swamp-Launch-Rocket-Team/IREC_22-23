#include "controller.h"
#include "../IMU/imu.h"

using namespace std;

int main()
{
    int address = 0x6B;

    imu_init(address);

    controller control("proto.cfg");

    state_t state;

    controller::motor_cmd_t motor_cmd;

    while (true)
    {
        state.imu_data = read_data();
        motor_cmd = control.control_loop(0,0,0,0,state);
        cout << motor_cmd.motor_1 << "    " << motor_cmd.motor_2 << "    " << motor_cmd.motor_3 << "    " << motor_cmd.motor_4 << "        ";
        usleep(10000);
        cout << "\r" << flush;
    }

    return 0;
}
