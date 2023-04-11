#pragma once
#include <string>
#include <termios>

class XBee
{
	private:
		int serial_port;
		struct termios tty; // ???
	public:
		XBee();
		~XBee();
		void transmit(char[] msg);
		std::string read();
}