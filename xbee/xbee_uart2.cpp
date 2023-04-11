// Dylan Ogrodowski
// Swamp Launch Rocket Team
// XBee UART interface
// 2/7/2023
#include "xbee_uart2.h"
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
/*class XBee()
{
	private:
		int serial_port;
		struct termios tty; // ???
	public:
		XBee();
		~XBee;
		void XBee::transmit(char[] msg);
		string XBee::read()
}*/

XBee::XBee()
{
	// UART setup source:l
	// https://medium.com/geekculture/raspberry-pi-c-libraries-for-working-with-i2c-spi-and-uart-4677f401b584
	serial_port = open("/dev/ttyUSB0", O_RDWR);
	if(tcgetattr(serial_port, &tty) != 0) {
		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
		return;
	}
	
	// UART Configuration flags
	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;
	tty.c_cflag &= ~CRTSCTS;
	tty.c_cflag |= CREAD | CLOCAL;
	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO;
	tty.c_lflag &= ~ECHOE;
	tty.c_lflag &= ~ECHONL;
	tty.c_lflag &= ~ISIG;
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
	tty.c_oflag &= ~OPOST;
	tty.c_oflag &= ~ONLCR;
	tty.c_cc[VTIME] = 10;
	tty.c_cc[VMIN] = 0;
	
	// Set the baudrate
	cfsetispeed(&tty, B9600);
	cfsetospeed(&tty, B9600);
	
	
	if (tcsetattr(serial_port, TCSANOW, &tty) != 0) 
	{
		printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
		return;
	}
}

XBee::~XBee()
{
	close(serial_port);
}

void XBee::transmit(char[] msg)
{
	write(serial_port, msg, sizeof(msg));
}

string XBee::read()
{
	char read_buf [256];
	memset(&read_buf, '\0', sizeof(read_buf);
	int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
	if (num_bytes < 0) 
	{
		printf("Error reading: %s", strerror(errno));
		return 1;
	}
	string s_b = convertToString(read_buf, num_bytes);
	return s_b;
}
