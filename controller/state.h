#ifndef STATE_H
#define STATE_H

#include "../IMU/imu.h"

struct state_t
{
    imu_data_t imu_data;

    // GPS DATA
    struct GPS_t
    {
        float x;
        float y;
    } gps;
    
    float altitude;
};

#endif
