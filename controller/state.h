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

    bool go_for_deployment = false;

    struct radio_command_t
    {
        int command_id = -1;

        enum operation_t
        {
            GO_FOR_DEPLOYMENT,
            STATE_TRANSITION,
            GPS_TARGET,
            MANUAL_CONTROL
        } operation;

        bool go_for_deployment;
        status_t new_status;
        float lat = 0;
        float lon = 0;
        float alt = 0;
        bool photo = false;
    } command_buffer;

    int highest_command_id_processed = 0;
};

#endif
