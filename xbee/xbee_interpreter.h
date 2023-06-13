#ifndef XBEE_INTERPRETER_H
#define XBEE_INTERPRETER_H

#include <thread>
#include <sstream>
#include <iomanip>

#include "../controller/state.h"
#include "xbee_uart2.h"

// Begin the thread for transmitting data via the XBee
void xbee_init(const state_t& state);

// Every second, transmits the following:
// Drone Status
// GPS
// Velocity
void xbee_transmit(const state_t& state);

#endif
