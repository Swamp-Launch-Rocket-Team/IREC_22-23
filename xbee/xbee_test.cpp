#include <iostream>
#include <unistd.h>
#include "xbee_uart2.h"

int main()
{
	XBee xbee;
	int i = 0;
	while(true)
	{
		std::cout << xbee.receive_line() << std::endl;
		std::string msg = "Raspberry Pi " + std::to_string(i) + "\n";
		xbee.transmit(msg);
		i++;
		usleep(100000);
	}
	return 0;
}
