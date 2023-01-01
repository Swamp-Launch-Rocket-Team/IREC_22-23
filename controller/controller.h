#pragma once

#include "PID.h"
#include "parse_cfg.h"
#include "state.h"
#include <cmath>

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
        struct motor_cmd_t
        {
            int motor_1;
            int motor_2;
            int motor_3;
            int motor_4;
        };

        controller();
        controller(std::string filename);
        ~controller();
        controller::motor_cmd_t control_loop(float x_setpoint, float y_setpoint, float z_setpoint, float yaw_setpoint, state_t &state);

};
