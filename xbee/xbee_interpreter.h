#ifndef XBEE_INTERPRETER_H
#define XBEE_INTERPRETER_H

#include "xbee_uart2.h"
#include "../controller/state.h"
#include "../controller/controller.h"

// Begin the thread for transmitting data via the XBee
void xbee_init(state_t& state);

// Every second, transmits telemetry and reads incoming commands
void xbee_thread(state_t& state);

// Transmits command acknowledgements, drone status, GPS, and velocity
void xbee_transmit(XBee& xbee, const state_t& state);

// Reads incoming data and interprets commands
void xbee_receive(XBee& xbee, state_t& state);

// Reads command from command buffer and alters the state and setpoint accordingly
void handle_xbee_command(state_t& state, setpoint_t& setpoint, bool print_log = false);

#endif
