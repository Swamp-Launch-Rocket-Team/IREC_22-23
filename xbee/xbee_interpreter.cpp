#include <thread>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <unistd.h>

#include "xbee_interpreter.h"

// Begin the thread for transmitting data via the XBee
void xbee_init(state_t& state)
{
    // Check if thread is already created
    static bool initialized = false;
    if (initialized)
    {
        return;
    }
    initialized = true;

    // Start thread
    std::thread sensor_thread(xbee_thread, std::ref(state));
    sensor_thread.detach();
}

// Every second, transmits telemetry and reads incoming commands
void xbee_thread(state_t& state)
{
    XBee xbee;

    while(true)
    {
        auto start = chrono::high_resolution_clock::now();
        xbee_transmit(xbee, state);
        do
        {
            xbee_receive(xbee, state);
            usleep(1000);
        }
        while(chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count() < 1000);
    }
}

// Transmits command acknowledgements, drone status, GPS, and velocity
void xbee_transmit(XBee xbee, const state_t& state)
{
    static int message_number = 1;

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
}

float hex_string_to_float(std::string hex_string)
{
    if(hex_string.length() != 8)
    {
        throw std::invalid_argument("string must be length 8");
    }
    for(char c : hex_string)
    {
        if(!isxdigit(c))
        {
            throw std::invalid_argument("string must only contain hexadecimal digits");
        }
    }
    uint32_t num;
    float f;
    sscanf(hex_string.c_str(), "%x", &num);
    f = *((float*)&num);
    return f;
}

// Reads incoming data and interprets commands
void xbee_receive(XBee xbee, state_t& state)
{
    std::string message = xbee.receive_message();
    if(message.length() < 5)
    {
        return;
    }

    // Parse command
    state_t::radio_command_t radio_command;
    if(message[0] == 'G') // Go for deployment command
    {
        radio_command.operation = state_t::radio_command_t::GO_FOR_DEPLOYMENT;
        if(message.length() != 5)
            return;

        if(message[4] == '0')
            radio_command.go_for_deployment = false;
        else if(message[4] == '1')
            radio_command.go_for_deployment = true;
        else
            return;
    }
    else if(message[0] == 'S') // State transition command
    {
        radio_command.operation = state_t::radio_command_t::STATE_TRANSITION;
        if(message.length() != 5)
            return;

        switch(message[4])
        {
            case '0':
                radio_command.new_status = state_t::ARMED;
                break;
            case '1':
                radio_command.new_status = state_t::LAUNCH;
                break;
            case '2':
                radio_command.new_status = state_t::EJECTION;
                break;
            case '3':
                radio_command.new_status = state_t::CONTAINER_RELEASE;
                break;
            case '4':
                radio_command.new_status = state_t::DEPLOYED;
                break;
            case '5':
                radio_command.new_status = state_t::PARACHUTE_RELEASE;
                break;
            case '6':
                radio_command.new_status = state_t::PARACHUTE_AVOIDANCE;
                break;
            case '7':
                radio_command.new_status = state_t::AUTONOMOUS;
                break;
            case '8':
                radio_command.new_status = state_t::DESCENT;
                break;
            case '9':
                radio_command.new_status = state_t::MANUAL;
                break;
            case 'A':
                radio_command.new_status = state_t::LANDED;
                break;
            default:
                return;
        }
    }
    else if(message[0] == 'T') // GPS target command
    {
        radio_command.operation = state_t::radio_command_t::GPS_TARGET;
        if(message.length() != 29)
            return;

        try
        {
            radio_command.lat = hex_string_to_float(message.substr(4, 8));
            radio_command.lon = hex_string_to_float(message.substr(4 + 8, 8));
            radio_command.alt = hex_string_to_float(message.substr(4 + 16, 8));
        }
        catch(const std::invalid_argument&)
        {
            return;
        }

        if(message[28] == '0')
            radio_command.photo = false;
        else if(message[28] == '1')
            radio_command.photo = true;
        else
            return;
    }
    else if(message[0] == 'M') // Manual control command
    {
        radio_command.operation = state_t::radio_command_t::MANUAL_CONTROL;
        if(message.length() != 13)
            return;

        float distance = 0;
        try
        {
            distance = hex_string_to_float(message.substr(5, 8));
        }
        catch(const std::invalid_argument&)
        {
            return;
        }

        const float meters_to_degrees = 8.98e-6;
        switch(message[4])
        {
            case 'N':
                radio_command.lat = distance * meters_to_degrees;
                break;
            case 'E':
                radio_command.lon = distance * meters_to_degrees;
                break;
            case 'S':
                radio_command.lat = - distance * meters_to_degrees;
                break;
            case 'W':
                radio_command.lon = - distance * meters_to_degrees;
                break;
            case 'U':
                radio_command.alt = distance;
                break;
            case 'D':
                radio_command.alt = - distance;
                break;
            default:
                return;
        }
    }
    else // Invalid opcode
    {
        return;
    }

    // Check command ID
    if(!isdigit(message[1]) || !isdigit(message[2]) || !isdigit(message[3]))
        return;
    try
    {
        radio_command.command_id = stoi(message.substr(1, 3));
    }
    catch(...)
    {
        return;
    }

    state.command_buffer = radio_command;
}

// Reads command from command buffer and alters the state and setpoint accordingly
void handle_xbee_command(state_t& state, setpoint_t& setpoint, bool print_log = false)
{
    // Ignore command if the command has already been handled
    if(state.command_buffer.command_id <= state.highest_command_id_processed)
        return;

    std::stringstream ss;
    ss << "ID: " << state.command_buffer.command_id << "\t";

    if(state.command_buffer.operation == state_t::radio_command_t::GO_FOR_DEPLOYMENT)
    {
        state.go_for_deployment = state.command_buffer.go_for_deployment;

        ss << "Go-for-deployment set to " << state.command_buffer.go_for_deployment;
    }
    else if(state.command_buffer.operation == state_t::radio_command_t::STATE_TRANSITION)
    {
        state.status = state.command_buffer.new_status;

        ss << "Change state to ";
        switch(state.command_buffer.new_status)
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
                // Cancels current movement by setting the new target to the drone's current position
                // TODO: Verify use of setpoint is correct.
                setpoint.x = state.imu_data.gps.lat;
                setpoint.y = state.imu_data.gps.lon;
                setpoint.z = state.imu_data.alt;
                // TODO: How does this handle the ultrasonic altitude?
                ss << "MANUAL";
                break;
            case state_t::LANDED:
                ss << "LANDED";
        }
    }
    else if(state.command_buffer.operation == state_t::radio_command_t::GPS_TARGET)
    {
        // TODO: Verify use of setpoint is correct.
        setpoint.x = state.command_buffer.lat;
        setpoint.y = state.command_buffer.lon;
        setpoint.z = state.command_buffer.alt;

        ss << "Set GPS target to ";
        ss << std::fixed << std::setprecision(5) << state.command_buffer.lat << ", ";
        ss << std::fixed << std::setprecision(5) << state.command_buffer.lon << ", ";
        ss << std::fixed << std::setprecision(2) << state.command_buffer.alt << ", ";
        ss << "photo: " << state.command_buffer.photo;
    }
    else if(state.command_buffer.operation == state_t::radio_command_t::MANUAL_CONTROL)
    {
        // TODO: Verify use of setpoint is correct.
        setpoint.x += state.command_buffer.lat;
        setpoint.y += state.command_buffer.lon;
        setpoint.z += state.command_buffer.alt;

        ss << "Change GPS target by ";
        ss << std::fixed << std::setprecision(5) << state.command_buffer.lat << ", ";
        ss << std::fixed << std::setprecision(5) << state.command_buffer.lon << ", ";
        ss << std::fixed << std::setprecision(2) << state.command_buffer.alt;
    }
    if(print_log)
        std::cout << ss.str() << std::endl;
    state.highest_command_id_processed = state.command_buffer.command_id;
}
