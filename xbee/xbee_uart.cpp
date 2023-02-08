#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

int main() 
{
	// UART setup source:
	// https://medium.com/geekculture/raspberry-pi-c-libraries-for-working-with-i2c-spi-and-uart-4677f401b584
	int serial_port = open("/dev/ttyUSB0", O_RDWR);
	struct termios tty;
	
	if(tcgetattr(serial_port, &tty) != 0) {
		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
		return 1;
	}
	
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
	
	cfsetispeed(&tty, B9600);
	cfsetospeed(&tty, B9600);
	
	
	if (tcsetattr(serial_port, TCSANOW, &tty) != 0) 
	{
		printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
		return 1;
	}
	
	unsigned char msg[] = {'H', 'e', 'l', 'l', 'o', '\r'};
	write(serial_port, msg, sizeof(msg));
	
	char read_buf [256];
	memset(&read_buf, '\0', sizeof(read_buf);
	int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
	if (num_bytes < 0) {
		printf("Error reading: %s", strerror(errno));
		return 1;
	}
	
	printf("Read %i bytes. Received message: %s", num_bytes, read_buf);
	close(serial_port);
	
	return 0;
}
	
	
void xbee_transmitt(char[] msg)
{
}

/*
ser = serial.Serial(
    port='/dev/ttyS0',
    baudrate = 9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1            
 )
 
 while 1:
	#ser.write(str.encode('Write counter: %d \n'%(counter)))
    #time.sleep(1)
    #counter += 1
	
x=ser.readline().strip()
print(x)
if x == 'a':
	GPIO.output(23,GPIO.HIGH)
	time.sleep(3)
*/