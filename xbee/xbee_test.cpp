#include <iostream>
#include <unistd.h>
#include <chrono>
#include "xbee_uart2.h"

int main()
{
	XBee xbee;
	int i = 0;
	while(true)
	{
		auto start = std::chrono::high_resolution_clock::now();
		std::cout << xbee.receive_message() << std::endl;
		std::string msg = "Raspberry Pi " + std::to_string(i) + "\n";
		xbee.transmit(msg);
		i++;
		while(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() < 2000);
	}
	return 0;
}
