#ifndef STATE_H
#define STATE_H

#include "../IMU/imu.h"

// Stores information of drone state
struct state_t
{
    imu_data_t imu_data;
    
    float ultra_alt = -1;

    enum status_t
    {
        ARMED,
        LAUNCH,
        EJECTION,
        CONTAINER_RELEASE,
        DEPLOYED,
        PARACHUTE_RELEASE,
        PARACHUTE_AVOIDANCE,
        AUTONOMOUS,
        DESCENT,
        MANUAL,
        LANDED
    } status;
};

#endif
