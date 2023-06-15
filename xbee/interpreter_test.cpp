#include <iostream>
#include "xbee_interpreter.h"

int main()
{
	state_t state;
	setpoint_t setpoint;
	xbee_init(state);

	// Main thread handles command queue
	while(true)
	{
		handle_xbee_command(state, setpoint, true);
		usleep(10000);
	}

	return 0;
}
