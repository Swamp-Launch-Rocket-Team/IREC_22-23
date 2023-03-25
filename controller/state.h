#ifndef STATE_H
#define STATE_H

#include "../IMU/imu.h"
#include "../IMU/imu.h"

// Stores information of drone state
struct state_t
{
    imu_data_t imu_data;
    
    float ultra_alt = -1;
};

#endif
