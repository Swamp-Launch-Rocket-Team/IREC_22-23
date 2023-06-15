#include <iostream>
#include <chrono>
#include "xbee_interpreter.h"

int main()
{
	state_t state;
	setpoint_t setpoint;
	xbee_init(state);

	// Main thread handles command queue
	while(true)
	{
		auto start = std::chrono::high_resolution_clock::now();
		handle_xbee_command(state, setpoint, true);
		while(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() < 9500);
	}

	return 0;
}
