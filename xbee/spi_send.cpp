// Dylan Ogrodowski
// 1/24/2023
// NOTE: OUTDATED FILE. SPI NO LONGER USED.

// https://www.digi.com/support/knowledge-base/an-introduction-to-spi-on-xbee-radios
// https://www.digi.com/resources/documentation/digidocs/pdfs/90002173.pdf "Select the SPI Port"
// Untested until SPI hardware connections are set up

//SOURCE: https://raw.githubusercontent.com/milekium/spidev-lib/master/sample/spidev-testcpp.cc

// TO DO: WIRING PI INCLUDE
// #include <>

using namespace std;

#define RESET ???
#define CHIP_SELECT_XBEE1 ???
#define ATTENTION ????

#define CHANNEL 1 // This corresponds to which CS is automatically driven, be aware to not use this pin for other purposes
#define BAUDRATE 1000000 // XBee supports baud rates up to 3.5 MHz, I suspect we will be limited by either camera or physical PCB layout noise constraints

#define XBEE_FRAME_START_DELIM

void xbee_init()
{
	// Initialize SPI
	
	
}

// Single-byte packet send
// Further optimization can be done to send multiple bytes per frame
// See https://www.digi.com/resources/documentation/digidocs/pdfs/90002173.pdf page 120 for further details
void xbee_send(uint8_t msg)
{
	// Calculate checksum
	uint8_t checksum = 0xFF - msg;
	
	digitalWrite(CHIP_SELECT_XBEE, 0);
	uint8_t buffer[5];
	buffer[0] = XBEE_FRAME_START_DELIM;
	buffer[1] = length_MSB;
	buffer[2] = length_LSB;
	buffer[3] = msg;
	buffer[4] = checksum;
	wiringPiSPIDataRW(CHANNEL, buffer, 5);
    digitalWrite(CHIP_SELECT_XBEE, 1);
}

// Poll the xbee to see if any new messages have been received
// If no new data is available, 0xFFFF is returned
// If new data is available, the data is read and the returned upper byte is 0x00 and the lower byte contains the read data
uint16_t xbee_poll()
{
	uint8_t atten = digitalRead(ATTENTION);
	if (!atten) // Attention is active low
	{
		digitalWrite(CHIP_SELECT_XBEE, 0);
		uint8_t buffer = 0xFF;
		buffer[0] = XBEE_FRAME_START_DELIM;
		buffer[1] = length_MSB;
		buffer[2] = length_LSB;
		buffer[3] = msg;
		buffer[4] = checksum;
		wiringPiSPIDataRW(CHANNEL, buffer, 5);
		digitalWrite(CHIP_SELECT_XBEE, 1);
	}
	else
	{
		return 0xFFFF; // No data received
	}
}
