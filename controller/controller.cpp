#include "controller.h"

using namespace std;

controller::controller(){}

controller::controller(string filename)
{
    cfg_data_t cfg_data = parse_cfg("proto.cfg");

    throttle = PID(cfg_data.throttle.kp, cfg_data.throttle.ki, cfg_data.throttle.kd);
    roll = PID(cfg_data.x.kp, cfg_data.x.ki, cfg_data.x.kd);
    pitch = PID(cfg_data.y.kp, cfg_data.y.ki, cfg_data.y.kd);
    yaw = PID(cfg_data.z.kp, cfg_data.z.ki, cfg_data.z.kd);
}

controller::~controller(){}

controller::motor_cmd_t controller::control_loop(float x_setpoint, float y_setpoint, float z_setpoint, float yaw_setpoint, state_t &state)
{
    controller::motor_cmd_t motor_cmd;
    
    float rel_x = 0; // Look into MATLAB code to figure this out
    float rel_y = 0; // Determined by dark magic!
    
    float roll_setpoint = y_pos.compute_PID(y_setpoint, rel_y); // should the setpoints be different? maybe not but also possibly
    float pitch_setpoint = x_pos.compute_PID(x_setpoint, rel_x);

    float roll_cmd = roll.compute_PID(roll_setpoint, state.imu_data.heading.x);
    float pitch_cmd = pitch.compute_PID(pitch_setpoint, state.imu_data.heading.y);
    float yaw_cmd = yaw.compute_PID(yaw_setpoint, state.imu_data.heading.z);
    float throttle_cmd = throttle.compute_PID(z_setpoint, state.altitude);

    motor_cmd.motor_1 = round(throttle_cmd + roll_cmd + pitch_cmd + yaw_cmd); // Double check this, this is copied from the matlab vid
    motor_cmd.motor_2 = round(throttle_cmd - roll_cmd + pitch_cmd - yaw_cmd);
    motor_cmd.motor_3 = round(throttle_cmd + roll_cmd - pitch_cmd - yaw_cmd);
    motor_cmd.motor_4 = round(throttle_cmd - roll_cmd - pitch_cmd + yaw_cmd);

    return motor_cmd;
}   