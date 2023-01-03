#ifndef STATE_H
#define STATE_H

#include "../IMU/imu.h"
#include "../IMU/imu.h"

// Stores information of drone state
struct state_t
{
    imu_data_t imu_data;

    axes_t velocity;

    // GPS DATA
    struct GPS_t
    {
        float x;
        float y;
    } gps;
    
    float altitude;
};

#endif
