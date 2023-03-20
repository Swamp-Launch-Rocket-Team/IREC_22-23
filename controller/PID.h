#pragma once

#include <chrono>
#include <queue>

class PID
{
    private:
        struct gains_t
        {
            float kp;
            float ki;
            float kd;
        } gains;

        float integral;
        queue<float> moving_int;
        float prev_error;
        std::chrono::high_resolution_clock::time_point prev_time;

    public:
        PID();
        PID(float kp, float ki, float kd);
        ~PID();
        float get_kp();
        float get_ki();
        float get_kd();
        float get_prev_error();
        float compute_PID(float setpoint, float currentpoint);
        float compute_PID(float error);
};
