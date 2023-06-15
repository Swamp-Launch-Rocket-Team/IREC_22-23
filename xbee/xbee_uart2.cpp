// Dylan Ogrodowski
// Swamp Launch Rocket Team
// XBee UART interface

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>

#include "xbee_uart2.h"

XBee::XBee()
{
	// UART setup source:l
	// https://medium.com/geekculture/raspberry-pi-c-libraries-for-working-with-i2c-spi-and-uart-4677f401b584
	serial_port = open("/dev/serial0", O_RDWR);
	if(tcgetattr(serial_port, &tty) != 0)
	{
		printf("%s:%d: Error %i from tcgetattr: %s\n", __FILE__, __LINE__ - 2, errno, strerror(errno));
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
		printf("%s:%d: Error %i from tcsetattr: %s\n", __FILE__, __LINE__ - 2, errno, strerror(errno));
		return;
	}
}

XBee::~XBee()
{
	close(serial_port);
}

ssize_t XBee::transmit(std::string msg)
{
	return write(serial_port, msg.c_str(), msg.length());
}

std::string convertToString(char* a, int size)
{
	int i;
	std::string s = "";
	for (i = 0; i < size; i++)
	{
		s = s + a[i];
	}
	return s;
}

std::string XBee::receive()
{
	char read_buf [256];
	memset(&read_buf, '\0', sizeof(read_buf));
	ssize_t num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
	if (num_bytes < 0)
	{
		printf("%s:%d: Error %i from read: %s\n", __FILE__, __LINE__ - 3, errno, strerror(errno));
		return "";
	}
	std::string s_b = convertToString(read_buf, num_bytes);
	return s_b;
}

// Blocks execution until a full message is received.
std::string XBee::receive_line()
{
	char read_buf [256];
	memset(&read_buf, '\0', sizeof(read_buf));
	ssize_t length = 0;
	do
	{
		ssize_t num_bytes = read(serial_port, read_buf + length, 1);
		if (num_bytes < 0)
		{
			printf("%s:%d: Error %i from read: %s\n", __FILE__, __LINE__ - 3, errno, strerror(errno));
		}
		else
		{
			length += num_bytes;
		}
	}
	while (read_buf[length - 1] != '\n');
	return convertToString(read_buf, length);
}

// Reads available data and adds it to a buffer.
// If the buffer contains a full message, it is returned.
// Otherwise, an empty string is returned.
std::string XBee::receive_message()
{
	// Stores incoming data across multiple function calls
	static std::string message_buf = "";

	// Read available data from device
	char read_buf [256];
	memset(&read_buf, '\0', sizeof(read_buf));
	ssize_t num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
	if (num_bytes < 0)
	{
		printf("%s:%d: Error %i from read: %s\n", __FILE__, __LINE__ - 3, errno, strerror(errno));
		return "";
	}
	std::string received_data = convertToString(read_buf, num_bytes);

	// Append new data to the buffer
	message_buf += received_data;

	// Find end of complete message
	std::size_t message_end = message_buf.find("\n");
	if(message_end == std::string::npos)
	{
		return "";
	}
    std::string complete_message = message_buf.substr(0, message_end);
    message_buf = message_buf.substr(message_end + 1);
    return complete_message;
}
