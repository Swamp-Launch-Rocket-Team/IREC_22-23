#include "dshot.h"

// https://brushlesswhoop.com/dshot-and-bidirectional-dshot/

// Timings in nanoseconds.
// Array index 1 represents logical 1. Array index 0 represents logical 0.
// First value in pair is time high. Second value is time low.
static std::pair<long, long> dshot_timings[2];

static uint8_t dshot_pin;

// static void send_bit(bool value);

void dshot_init(uint8_t pin_number, dshot_standard_t standard)
{
	// Set up output pin
	wiringPiSetup();
	dshot_pin = pin_number;
	pinMode(dshot_pin, OUTPUT);

	// Set up timings
	int bit_length;
	switch(standard)
	{
		case DSHOT1200:
			bit_length = 83;
			dshot_timings[1].first = 63;
			dshot_timings[0].first = 31;
			break;
		case DSHOT600:
			bit_length = 167;
			dshot_timings[1].first = 125;
			dshot_timings[0].first = 63;
			break;
		case DSHOT300:
			bit_length = 333;
			dshot_timings[1].first = 250;
			dshot_timings[0].first = 125;
			break;
		default:
			bit_length = 667;
			dshot_timings[1].first = 500;
			dshot_timings[0].first = 250;
	}
	dshot_timings[1].second = bit_length - dshot_timings[1].first;
	dshot_timings[0].second = bit_length - dshot_timings[0].first;
}

void dshot_send(uint16_t throttle)
{
	// Error if not initialized
	if(!dshot_timings[0].first)
	{
		throw std::runtime_error("Dshot not initialized");
	}

	// Max throttle is 1999
	if(throttle > 1999)
	{
		throttle = 1999;
	}

	// Throttle ranges from 48-2047
	throttle += 48;

	// Append telemetry request bit
	uint16_t message = throttle << 1;

	// Append cyclic redundancy check (CRC)
	uint8_t crc = (message ^ (message >> 4) ^ (message >> 8)) & 0x0F;

	message = message << 4 | crc;

	// printf("0x%X\n", message);

	// Send bits
	for(int8_t bit = 15; bit >= 0; bit--)
	{
		send_bit((message >> bit) & 1);
	}
}

void send_bit(bool value)
{
	// Time high
	digitalWrite(dshot_pin, 1);
	busy10ns(dshot_timings[value].first);

	// Time low
	digitalWrite(dshot_pin, 0);
	busy10ns(dshot_timings[value].second);
}
