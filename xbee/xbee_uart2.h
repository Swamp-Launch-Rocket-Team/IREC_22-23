#pragma once
#include <string>
#include <termios.h>

class XBee
{
	private:
		int serial_port;
		struct termios tty;
	public:
		XBee();
		~XBee();
		ssize_t transmit(std::string msg);
		std::string receive();
		std::string receive_line();
		std::string receive_message();
};
