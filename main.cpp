#include <iostream>
#include <chrono>
#include <list>
#include <fstream>

#include "IMU/imu.h"
#include "DShot/dshot.h"
#include "controller/controller.h"
#include "DShot/busynano/busynano.h"
// TODO: add buzzer, camera, ultrasonic, and xbee headers

bool send_motor_cmds(motor_cmd_t &cmd, vector<Dshot> &motors);

using namespace std;

int main()
{
    // Initialize all subsystems
    // Initialize IMU
    int file;
    int imu_address = 0x6B;
    file = imu_init(imu_address);

    // Initialize Dshot
    vector<Dshot> motors;
    motors.emplace_back(26);
    motors.emplace_back(27);
    motors.emplace_back(28);
    motors.emplace_back(29);
    Dshot::set_speed_standard(Dshot::DSHOT600);

    state_t state; // Stores information of drone state
    controller control("proto.cfg"); // Controller class, stores PID loop for each controller
    setpoint_t setpoint;
    list<pair<long, state_t>> data_log;

    // Idle on pad

    // Idle in flight

    // Container release

    // Await release command

    // Await altitude

    // Flight
    auto start = chrono::high_resolution_clock::now();
    auto cur = chrono::high_resolution_clock::now();
    list<pair<float, motor_cmd_t>> cmd_log;
    group_startup(motors);
    while (chrono::duration_cast<chrono::seconds>(cur - start).count() < 60)
    {
        cur = chrono::high_resolution_clock::now();
        // Square Wave
        if (chrono::duration_cast<chrono::milliseconds>(cur - start).count() % 10000 <= 5000)
        {
            setpoint.x = 20; // degrees
        }
        else
        {
            setpoint.x = -20; // degrees
        }

        // Sine Wave
        // setpoint.x = 30 * sin(chrono::duration_cast<chrono::microseconds>(cur - start).count()/1000000.0);
        // if (setpoint.x > 0)
        // {
        //     setpoint.x = 180 - setpoint.x;
        // }
        // else
        // {
        //     setpoint.x = -180 - setpoint.x;
        // }

        // Integral Wind-Up Test
        // if (chrono::duration_cast<chrono::milliseconds>(cur - start).count() <= 50000)
        // {
        //     setpoint.x = 20; // degrees
        // }
        // else
        // {
        //     setpoint.x = -20; // degrees
        // }


        state.imu_data = imu_read_data();
        motor_cmd_t motor_cmd = control.control_loop(setpoint, state);
        // cout << setpoint.x << " " << state.imu_data.heading.x << endl;

        int delay = 500000;
        busy10ns(delay); // 5 ms delay
        send_motor_cmds(motor_cmd, motors);

        data_log.push_back(make_pair(chrono::duration_cast<chrono::microseconds>(cur - start).count(), state));
        cmd_log.push_back(make_pair(setpoint.x, motor_cmd));

        while (chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - start).count() < 9500);
    }

    // Descent

    // Shutdown
    // Record log to file
    ofstream log_file;
    log_file.open("log.csv");

    log_file << "timestamp,"
            << "roll,"
            << "pitch,"
            << "yaw,"
            << "vel_x,"
            << "vel_y,"
            << "vel_z,"
            << "ang_vel_x,"
            << "ang_vel_y,"
            << "ang_vel_z,"
            << "gps_lat,"
            << "gps_lon,"
            << "imu_alt,"
            << "ultrasonic_alt,"
            << "command,"
            << "motor_1,"
            << "motor_2,"
            << "motor_3,"
            << "motor_4" << endl;

    auto it2 = cmd_log.begin();
    for (auto it1 = data_log.begin(); it1 != data_log.end(); it1++, it2++)
    {
        log_file << ((*it1).first) << ","
            << (*it1).second.imu_data.heading.x << ","
            << (*it1).second.imu_data.heading.y << ","
            << (*it1).second.imu_data.heading.z << ","
            << (*it1).second.imu_data.velocity.x << ","
            << (*it1).second.imu_data.velocity.y << ","
            << (*it1).second.imu_data.velocity.z << ","
            << (*it1).second.imu_data.ang_v.x << ","
            << (*it1).second.imu_data.ang_v.y << ","
            << (*it1).second.imu_data.ang_v.z << ","
            << (*it1).second.imu_data.gps.lat << ","
            << (*it1).second.imu_data.gps.lon << ","
            << (*it1).second.imu_data.alt << ","
            << (*it1).second.ultra_alt << ","
            << (*it2).first << ","
            << (*it2).second.motor_1 << ","
            << (*it2).second.motor_2 << ","
            << (*it2).second.motor_3 << ","
            << (*it2).second.motor_4 << endl;
    }
    log_file.close();

    return 0;
}

bool send_motor_cmds(motor_cmd_t &cmd, vector<Dshot> &motors)
{
    motors[0].throttle(cmd.motor_1);
    motors[1].throttle(cmd.motor_2);
    motors[2].throttle(cmd.motor_3);
    motors[3].throttle(cmd.motor_4);
    return true;
}
