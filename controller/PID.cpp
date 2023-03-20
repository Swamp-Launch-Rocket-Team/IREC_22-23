#include "PID.h"

using namespace std;

PID::PID()
{
    gains.kp = 0;
    gains.ki = 0;
    gains.kd = 0;

    integral = 0;
    prev_error = 0;

    prev_time = chrono::high_resolution_clock::now();
}

PID::PID(float kp, float ki, float kd)
{
    gains.kp = kp;
    gains.ki = ki;
    gains.kd = kd;

    integral = 0;
    prev_error = 0;
    for (int i = 0; i < 2000; ++i)
    {
        moving_int.push(0);
    }

    prev_time = chrono::high_resolution_clock::now();
}

PID::~PID() {}

float PID::get_kp()
{
    return this->gains.kp;
}

float PID::get_ki()
{
    return this->gains.ki;
}

float PID::get_kd()
{
    return this->gains.kd;
}

float PID::get_prev_error()
{
    return this->prev_error;
}

float PID::compute_PID(float setpoint, float currentpoint)
{
    float cur_error = setpoint - currentpoint;

    auto cur_time = chrono::high_resolution_clock::now();

    float dt = chrono::duration_cast<chrono::microseconds>(cur_time - prev_time).count();

    float P = gains.kp * cur_error;

    integral -= moving_int.front();
    moving_int.pop();
    moving_int.push(cur_error * dt);
    integral += moving_int.back();

    float I = gains.ki * integral;

    float D = gains.kd * (cur_error - prev_error) / dt;

    prev_time = cur_time;

    prev_error = cur_error;

    return (P + I + D);
}

float PID::compute_PID(float error)
{
    auto cur_time = chrono::high_resolution_clock::now();

    float dt = chrono::duration_cast<chrono::microseconds>(cur_time - prev_time).count();

    float P = gains.kp * error;

    integral -= moving_int.front();
    moving_int.pop();
    moving_int.push(error * dt);
    integral += moving_int.back();

    float I = gains.ki * integral;

    float D = gains.kd * (error - prev_error) / dt;

    prev_time = cur_time;

    prev_error = error;

    return (P + I + D);
}
