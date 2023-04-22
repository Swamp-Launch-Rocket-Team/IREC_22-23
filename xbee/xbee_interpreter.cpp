#include "xbee_interpreter.h"
#include <unistd.h>

// Begin the thread for transmitting data via the XBee
void xbee_init(const state_t& state)
{
    static bool initialized = false;
    if (initialized)
    {
        return;
    }
    initialized = true;

    // Start thread
    std::thread sensor_thread(xbee_transmit, std::cref(state));
    sensor_thread.detach();
}

// Every second, transmits the following:
// Drone Status
// GPS
// Velocity
void xbee_transmit(const state_t& state)
{
    XBee xbee;
    int message_number = 0;
    while(true)
    {
        std::string msg = "";
        switch(state.status)
        {
            case state_t::ARMED:
                msg = "ARMED";
                break;
            case state_t::LAUNCH:
                msg = "LAUNCH";
                break;
            case state_t::EJECTION:
                msg = "EJECTION";
                break;
            case state_t::CONTAINER_RELEASE:
                msg = "CONTAINER_RELEASE";
                break;
            case state_t::DEPLOYED:
                msg = "DEPLOYED";
                break;
            case state_t::PARACHUTE_RELEASE:
                msg = "PARACHUTE_RELEASE";
                break;
            case state_t::PARACHUTE_AVOIDANCE:
                msg = "PARACHUTE_AVOIDANCE";
                break;
            case state_t::AUTONOMOUS:
                msg = "AUTONOMOUS";
                break;
            case state_t::DESCENT:
                msg = "DESCENT";
                break;
            case state_t::MANUAL:
                msg = "MANUAL";
        }
        msg = msg + ", "
            + to_string(state.imu_data.gps.lat) + ", "
            + to_string(state.imu_data.gps.lon) + ","
            + to_string(state.imu_data.velocity.x) + ","
            + to_string(state.imu_data.velocity.y) + ","
            + to_string(state.imu_data.velocity.z) + ",";
		xbee.transmit(msg);
        message_number++;
        usleep(1000000);
    }
}
