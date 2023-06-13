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
    static int message_number = 1;
    while(true)
    {
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0') << state.highest_command_id_processed << ", ";
        ss << std::setw(5) << std::setfill('0') << message_number << ", ";
        switch(state.status)
        {
            case state_t::ARMED:
                ss << "ARMED";
                break;
            case state_t::LAUNCH:
                ss << "LAUNCH";
                break;
            case state_t::EJECTION:
                ss << "EJECTION";
                break;
            case state_t::CONTAINER_RELEASE:
                ss << "CONTAINER_RELEASE";
                break;
            case state_t::DEPLOYED:
                ss << "DEPLOYED";
                break;
            case state_t::PARACHUTE_RELEASE:
                ss << "PARACHUTE_RELEASE";
                break;
            case state_t::PARACHUTE_AVOIDANCE:
                ss << "PARACHUTE_AVOIDANCE";
                break;
            case state_t::AUTONOMOUS:
                ss << "AUTONOMOUS";
                break;
            case state_t::DESCENT:
                ss << "DESCENT";
                break;
            case state_t::MANUAL:
                ss << "MANUAL";
                break;
            case state_t::LANDED:
                ss << "LANDED";
        }
        ss << ", " << std::fixed << std::setprecision(5) << state.imu_data.gps.lat;
        ss << ", " << std::fixed << std::setprecision(5) << state.imu_data.gps.lon;
        ss << ", " << std::fixed << std::setprecision(3) << state.imu_data.velocity.x;
        ss << ", " << std::fixed << std::setprecision(3) << state.imu_data.velocity.y;
        ss << ", " << std::fixed << std::setprecision(3) << state.imu_data.velocity.z;
        ss << "\n";
		xbee.transmit(ss.str());
        message_number++;
        usleep(1000000);
    }
}
