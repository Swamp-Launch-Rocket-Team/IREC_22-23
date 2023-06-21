#include <iostream>
#include <chrono>
#include <list>
#include <fstream>
#include <stdlib.h>
#include <filesystem>
#include <thread>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "IMU/imu.h"
#include "DShot/dshot.h"
#include "controller/controller.h"
#include "DShot/busynano/busynano.h"
#include "ultrasonic/ultrasonic.h"
#include "buzzer/buzzer.h"
#include "xbee/xbee_interpreter.h"

void armed_status(vector<Dshot> &motors, state_t &state, pair<long, state_t> (&launch_detect_log)[1024], int &index, chrono::_V2::system_clock::time_point &start, chrono::_V2::system_clock::time_point &launch_time);
void launch_status(vector<Dshot> &motors, state_t &state, chrono::_V2::system_clock::time_point &launch_time);
void eject_status(vector<Dshot> &motors, state_t &state);
void container_release(vector<Dshot> &motors, state_t &state, chrono::_V2::system_clock::time_point &motor_start);
void deploy_status(vector<Dshot> &motors, state_t &state, chrono::_V2::system_clock::time_point &launch_time);
void parachute_release(vector<Dshot> &motors, state_t &state, chrono::_V2::system_clock::time_point &motor_start);
void parachute_avoid(vector<Dshot> &motors, state_t &state, controller &control, setpoint_t &setpoint);
void auto_status(vector<Dshot> &motors, state_t &state, controller &control, setpoint_t &setpoint);
void descent_status(vector<Dshot> &motors, state_t &state, controller &control, setpoint_t &setpoint);
void manual_status(vector<Dshot> &motors, state_t &state, controller &control, setpoint_t &setpoint);
void landed_status(state_t &state, list<pair<long, state_t>> &data_log, list<pair<float, motor_cmd_t>> &cmd_log);

bool send_motor_cmds(motor_cmd_t &cmd, vector<Dshot> &motors);
float axes_mag(axes_t &axes);
void write_data(list<pair<long, state_t>> &data_log, list<pair<float, motor_cmd_t>> &cmd_log);
bool detect_launch(pair<long, state_t> (&launch_detect_log)[1024], int index);

using namespace std;

int main(int argc, char* argv[])
{
    // Set up camera lambda function
    auto take_photo = [argv]()
    {
        std::thread t([&]() {
            const std::filesystem::path relative_path_to_camera_script = "camera.py";

            // Get absolute path of camera python script
            std::filesystem::path path = argv[0];
            if(path.is_relative())
            {
                path = std::string(argv[0]).substr(2);
            }
            path = std::filesystem::absolute(path).parent_path();
            path = path / relative_path_to_camera_script;

            // Run camera python script
            std::string script = "python " + path.string() + " &";
            system(script.c_str());
        });
        t.detach();
        return t;
    };

    // Turn off buzzer
    wiringPiSetup();
    pinMode(21, OUTPUT);
    digitalWrite(21, LOW);

    // Initialize all subsystems
    // Initialize IMU
    int file;
    int imu_address = 0x6B;
    file = imu_init(imu_address);
    // Initialize ultrasonic
    ultrasonic_init(3, 2);

    // Initialize servos
    pinMode(6, OUTPUT); // container motors

    // Initialize Dshot
    vector<Dshot> motors;
    motors.emplace_back(28);
    motors.emplace_back(29);
    motors.emplace_back(26);
    motors.emplace_back(27);
    Dshot::set_speed_standard(Dshot::DSHOT600);

    state_t state; // Stores information of drone state
    controller control("proto.cfg"); // Controller class, stores PID loop for each controller
    setpoint_t setpoint;
    list<pair<long, state_t>> data_log;
    pair<long, state_t> launch_detect_log[1024]; // Is this allowed?
    int launch_detect_log_index = 0;

    auto start = chrono::high_resolution_clock::now();
    auto cur = chrono::high_resolution_clock::now();
    auto launch_time = chrono::high_resolution_clock::now();
    auto container_motor_start = chrono::high_resolution_clock::now();
    auto parachute_motor_start = chrono::high_resolution_clock::now();
    list<pair<float, motor_cmd_t>> cmd_log;
    motor_cmd_t motor_cmd = control.set_zero();
    // group_startup(motors);

    state.status = state_t::ARMED;
    bool camera = false;
    bool container_motor = false;
    bool parachute_motor = false;

    // TODO: BEEPS!

    xbee_init(state);

    while (true && state.status != state_t::LANDED)
    {
        cur = chrono::high_resolution_clock::now();
        state.imu_data = imu_read_data();
        state.ultra_alt = ultrasonic_read();

        if (state.imu_data.pressure == 0 && !data_log.empty())
        {
            state.imu_data.pressure = data_log.back().second.imu_data.pressure;
        }

        switch (state.status)
        {
        case state_t::ARMED:
            armed_status(motors, state, launch_detect_log, launch_detect_log_index, start, launch_time);
            break;

        case state_t::LAUNCH:
            launch_status(motors, state, launch_time);
            break;

        case state_t::EJECTION:
            eject_status(motors, state);
            break;

        case state_t::CONTAINER_RELEASE:
            if (!container_motor)
            {
                container_motor_start = chrono::high_resolution_clock::now();
                container_motor = true;
            }
            container_release(motors, state, container_motor_start);
            break;

        case state_t::DEPLOYED:
            deploy_status(motors, state, launch_time);
            break;

        case state_t::PARACHUTE_RELEASE:
            if (!parachute_motor)
            {
                parachute_motor_start = chrono::high_resolution_clock::now();
                parachute_motor = true;
            }
            parachute_release(motors, state, parachute_motor_start);
            break;

        case state_t::PARACHUTE_AVOIDANCE:
            parachute_avoid(motors, state, control, setpoint);
            break;

        case state_t::AUTONOMOUS:
            auto_status(motors, state, control, setpoint);
            break;

        case state_t::DESCENT:
            descent_status(motors, state, control, setpoint);
            break;

        case state_t::MANUAL:
            manual_status(motors, state, control, setpoint);
            break;
        }

        data_log.push_back(make_pair(chrono::duration_cast<chrono::microseconds>(cur - start).count(), state));
        cmd_log.push_back(make_pair(setpoint.y, motor_cmd));

        if (state.status != state_t::ARMED && !camera)
        {
            string script = "python camera/camera.py &"; // start camera
            system(script.c_str());

            launch_time = chrono::high_resolution_clock::now();

            camera = true;
        }

        handle_xbee_command(state, setpoint);

        while (chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - cur).count() < 9500);
    }

    landed_status(state, data_log, cmd_log);

    return 0;
}

void armed_status(vector<Dshot> &motors, state_t &state, pair<long, state_t> (&launch_detect_log)[1024], int &index, chrono::_V2::system_clock::time_point &start, chrono::_V2::system_clock::time_point &launch_time)
{
    launch_detect_log[index & 1023] = make_pair(chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - start).count(), state);

    if (detect_launch(launch_detect_log, index))
    {
        state.status = state_t::LAUNCH;
    }

    index++;

    return;
}

void launch_status(vector<Dshot> &motors, state_t &state, chrono::_V2::system_clock::time_point &launch_time)
{
    // TODO: DOUBLE CHECK THIS TIMER

    if (chrono::duration_cast<chrono::seconds>(chrono::high_resolution_clock::now() - launch_time).count() > 140)
    {
        state.status = state_t::EJECTION;
    }

    return;
}

void eject_status(vector<Dshot> &motors, state_t &state)
{
    // TODO: WHAT DO WE DO HERE????

    state.status = state_t::CONTAINER_RELEASE;

    return;
}

void container_release(vector<Dshot> &motors, state_t &state, chrono::_V2::system_clock::time_point &motor_start)
{
    digitalWrite(6, HIGH);

    if (chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - motor_start).count() > 1500)
    {
        digitalWrite(6, LOW);
        state.status = state_t::DEPLOYED;
    }

    return;
}

void deploy_status(vector<Dshot> &motors, state_t &state, chrono::_V2::system_clock::time_point &launch_time)
{
    if (chrono::duration_cast<chrono::seconds>(chrono::high_resolution_clock::now() - launch_time).count() > 5 * 60)
    {
        state.status = state_t::LANDED;
    }

    return;
}

void parachute_release(vector<Dshot> &motors, state_t &state, chrono::_V2::system_clock::time_point &motor_start)
{
    // int motor_gpio = 5;

    // pinMode(motor_gpio, OUTPUT);
    // digitalWrite(motor_gpio, HIGH);

    // busy10ns(100 * 1000 * 50); // wait 0.5 seconds TODO: TEST THIS

    // digitalWrite(motor_gpio, LOW);

    // state.status = state_t::PARACHUTE_AVOIDANCE;
    state.status = state_t::DEPLOYED;

    return;
}

void parachute_avoid(vector<Dshot> &motors, state_t &state, controller &control, setpoint_t &setpoint)
{
    state.status = state_t::DEPLOYED;
    return;
}

void auto_status(vector<Dshot> &motors, state_t &state, controller &control, setpoint_t &setpoint)
{
    state.status = state_t::DEPLOYED;
    return;
}

void descent_status(vector<Dshot> &motors, state_t &state, controller &control, setpoint_t &setpoint)
{
    state.status = state_t::DEPLOYED;
    return;
}

void manual_status(vector<Dshot> &motors, state_t &state, controller &control, setpoint_t &setpoint)
{
    state.status = state_t::DEPLOYED;
    return;
}

void landed_status(state_t &state, list<pair<long, state_t>> &data_log, list<pair<float, motor_cmd_t>> &cmd_log)
{
    write_data(data_log, cmd_log);

    digitalWrite(21, HIGH); // buzzer scream!

    // TODO: edit to shutdown after some amount of time
    while (true)
    {
        sleep(1);
    }

    return;
}


// Miscellaneous functions

bool detect_launch(pair<long, state_t> (&launch_detect_log)[1024], int index)
{
    float sorted_log[300];

    // for (int i = 0; i < 300; ++i)
    // {
    //     sorted_log[i] = axes_mag(launch_detect_log[(index - (300 + i)) & 1023].second.imu_data.accel);
    // }

    // for (int i = 0; i < 300; ++i)
    // {
    //     for (int j = 0; j < 300; ++j)
    //     {
    //         if (sorted_log[j] > sorted_log[i + 1])
    //         {
    //             float temp = sorted_log[j];
    //             sorted_log[j] = sorted_log[i + 1];
    //             sorted_log[i + 1] = temp;
    //         }
    //     }
    // }

    // float avg_accel = 0;

    // for (int i = 10; i < 290; ++i)
    // {
    //     avg_accel += sorted_log[i];
    // }

    // avg_accel /= 280;

    // if (avg_accel > 5 * 9.8) // 5 gs of acceleration
    // {
    //     return true;
    // }

    float p_avg_accel = 0;

    for (int i = 0; i < 300; ++i)
    {
        p_avg_accel += axes_mag(launch_detect_log[(index - (300 + i)) & 1023].second.imu_data.accel);
    }

    p_avg_accel /= 300;

    float stdev = 0;

    for (int i = 0; i < 300; ++i)
    {
        stdev += pow(axes_mag(launch_detect_log[(index - (300 + i)) & 1023].second.imu_data.accel) - p_avg_accel, 2.0);
    }

    stdev = sqrt(stdev / 299);

    float avg_accel = 0;
    int count;

    for (int i = 0; i < 300; ++i)
    {
        if (axes_mag(launch_detect_log[(index - (300 + i)) & 1023].second.imu_data.accel) < 3 * stdev)
        {
            avg_accel += axes_mag(launch_detect_log[(index - (300 + i)) & 1023].second.imu_data.accel);
            count++;
        }
    }

    avg_accel /= count;

    if (avg_accel > 5 * 9.8) // 5 gs of acceleration
    {
        return true;
    }

    return false;
}

bool send_motor_cmds(motor_cmd_t &cmd, vector<Dshot> &motors)
{
    motors[0].throttle(cmd.motor_1);
    motors[1].throttle(cmd.motor_2);
    motors[2].throttle(cmd.motor_3);
    motors[3].throttle(cmd.motor_4);
    return true;
}

float axes_mag(axes_t &axes)
{
    return sqrt(axes.x * axes.x + axes.y * axes.y + axes.z * axes.z);
}

void write_data(list<pair<long, state_t>> &data_log, list<pair<float, motor_cmd_t>> &cmd_log)
{
    time_t datetime = time(nullptr);
    tm local_datetime = *localtime(&datetime);

    ostringstream oss;
    oss << put_time(&local_datetime, "%Y%m%d_%H-%M-%S.csv");
    string filename = oss.str();

    ofstream log_file;
    log_file.open(filename);

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
            << "accel_x,"
            << "accel_y,"
            << "accel_z,"
            << "gps_lat,"
            << "gps_lon,"
            << "imu_alt,"
            << "ultrasonic_alt,"
            << "pressure,"
            << "status,"
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
            << (*it1).second.imu_data.accel.x << ","
            << (*it1).second.imu_data.accel.y << ","
            << (*it1).second.imu_data.accel.z << ","
            << (*it1).second.imu_data.gps.lat << ","
            << (*it1).second.imu_data.gps.lon << ","
            << (*it1).second.imu_data.alt << ","
            << (*it1).second.ultra_alt << ","
            << (*it1).second.imu_data.pressure << ","
            << (*it1).second.status << ","
            << (*it2).first << ","
            << (*it2).second.motor_1 << ","
            << (*it2).second.motor_2 << ","
            << (*it2).second.motor_3 << ","
            << (*it2).second.motor_4 << endl;
    }

    log_file.close();

    return;
}
