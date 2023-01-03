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

    public:
        controller();
        controller(std::string filename);
        // ~controller();
        motor_cmd_t control_loop(float x_setpoint, float y_setpoint, float z_setpoint, float yaw_setpoint, state_t &state);

};
