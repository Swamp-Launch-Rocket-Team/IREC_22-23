#pragma once
#include <string>

class XBee()
{
	private:
		int serial_port;
		struct termios tty; // ???
	public:
		XBee();
		~XBee;
		void XBee::transmit(char[] msg);
		string XBee::read();
}