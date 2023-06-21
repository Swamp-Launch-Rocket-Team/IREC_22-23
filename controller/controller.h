#pragma once

#include "PID.h"
#include "parse_cfg.h"
#include "state.h"
#include <cmath>

// Struct for storing motor RPM commands
struct motor_cmd_t
{
    int motor_1;
    int motor_2;
    int motor_3;
    int motor_4;
};

struct setpoint_t
{
    float x;
    float y;
    float z;
    float yaw;
};

// Controller class, stores PID loop for each controller
class controller
{
    private:
        PID x_pos;
        PID y_pos;
        PID throttle;
        PID roll;
        PID pitch;
        PID yaw;
        float d_gps[100][2];
        int count = 0;
        chrono::high_resolution_clock::time_point prev_time;
        float prev_lat = -1;
        float prev_lon = -1;

    public:
        controller();
        controller(std::string filename);
        // ~controller();
        motor_cmd_t control_loop(setpoint_t &setpoint, state_t &state);
        motor_cmd_t set_zero();

};
