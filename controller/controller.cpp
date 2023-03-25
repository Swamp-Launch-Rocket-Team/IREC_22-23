#include "controller.h"

#define PI 3.14159265358979323846

using namespace std;

controller::controller(){}

// Controller constructor
// \param filename: name of *.cfg file
controller::controller(string filename)
{
    cfg_data_t cfg_data = parse_cfg(filename);

    throttle = PID(cfg_data.throttle.kp, cfg_data.throttle.ki, cfg_data.throttle.kd);
    roll = PID(cfg_data.x.kp, cfg_data.x.ki, cfg_data.x.kd);
    pitch = PID(cfg_data.y.kp, cfg_data.y.ki, cfg_data.y.kd);
    yaw = PID(cfg_data.z.kp, cfg_data.z.ki, cfg_data.z.kd);
    x_pos = PID(cfg_data.x_pos.kp, cfg_data.x_pos.ki, cfg_data.x_pos.kd);
    y_pos = PID(cfg_data.y_pos.kp, cfg_data.y_pos.ki, cfg_data.y_pos.kd);
}

// Calculate motor throttles
// \param x_setpoint: desired x-coord target
// \param y_setpoint: desired y-coord target
// \param z_setpoint desired z-coord target
// \param state: current state of IMU
// \return motor_cmd_t struct with motor throttle commands
motor_cmd_t controller::control_loop(setpoint_t &setpoint, state_t &state)
{
    motor_cmd_t motor_cmd;

    // Make sure the signs on this entire function are correct
    float rel_x = cos(state.imu_data.heading.z * PI / 180); // Look into MATLAB code to figure this out
    float rel_y = sin(state.imu_data.heading.z * PI / 180); // Determined by dark magic!

    float roll_setpoint = y_pos.compute_PID(setpoint.y, rel_y); // should the setpoints be different? maybe not but also possibly
    // float pitch_setpoint = x_pos.compute_PID(setpoint.x, rel_x);

    float pitch_setpoint = setpoint.x;

    float roll_error = roll_setpoint - state.imu_data.heading.x;
    float pitch_error = pitch_setpoint - state.imu_data.heading.y;
    float yaw_error = setpoint.yaw - state.imu_data.heading.z;

    if (roll_error > 180)
    {
        roll_error = -(360 - roll_error);
    }
    else if (roll_error < -180)
    {
        roll_error = 360 + roll_error;
    }

    if (pitch_error > 180)
    {
        pitch_error = -(360 - pitch_error);
    }
    else if (roll_error < -180)
    {
        pitch_error = 360 + pitch_error;
    }

    if (yaw_error > 180)
    {
        yaw_error = -(360 - yaw_error);
    }
    else if (yaw_error < -180)
    {
        yaw_error = 360 + yaw_error;
    }

    float roll_cmd = roll.compute_PID(roll_error);
    float pitch_cmd = pitch.compute_PID(pitch_error);
    float yaw_cmd = yaw.compute_PID(yaw_error); // How do we make the yaw take the shortest path to target? Ex: state -179' target +179', we want it to go -2', not +358'.
                                                                             // Possible solutions: give it the distance to yaw_setpoint instead of setpoint itself. Ex: Pass in -2' instead of +179'
                                                                             // We would need to potentially write a thing to make sure the yaw wraps around correctly? (maybe not though)
    float throttle_cmd = throttle.compute_PID(setpoint.z, state.imu_data.alt);

                                                                              // from top:
    // motor_cmd.motor_1 = round(throttle_cmd + roll_cmd + pitch_cmd + yaw_cmd); // front left
    // motor_cmd.motor_2 = round(throttle_cmd - roll_cmd + pitch_cmd - yaw_cmd); // front right
    // motor_cmd.motor_3 = round(throttle_cmd + roll_cmd - pitch_cmd - yaw_cmd); // back left
    // motor_cmd.motor_4 = round(throttle_cmd - roll_cmd - pitch_cmd + yaw_cmd); // back right


    motor_cmd.motor_1 = round(pitch_cmd); // front left
    motor_cmd.motor_2 = round(pitch_cmd); // front right
    motor_cmd.motor_3 = round(- pitch_cmd); // back left
    motor_cmd.motor_4 = round(- pitch_cmd); // back right

    if (motor_cmd.motor_1 < 0)
    {
        motor_cmd.motor_1 = 0;
    }
    if (motor_cmd.motor_2 < 0)
    {
        motor_cmd.motor_2 = 0;
    }
    if (motor_cmd.motor_3 < 0)
    {
        motor_cmd.motor_3 = 0;
    }
    if (motor_cmd.motor_4 < 0)
    {
        motor_cmd.motor_4 = 0;
    }

    motor_cmd.motor_1 += 100;
    motor_cmd.motor_2 += 100;
    motor_cmd.motor_3 += 100;
    motor_cmd.motor_4 += 100;

    return motor_cmd;
}
