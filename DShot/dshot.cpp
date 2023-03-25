#include "dshot.h"

// https://brushlesswhoop.com/dshot-and-bidirectional-dshot/

// static void send_bit(bool value);

std::pair<long, long> Dshot::dshot_timings[2];

Dshot::Dshot(uint8_t pin_number)
{
	// Set up output pin
	wiringPiSetup();
	dshot_pin = pin_number;
	pinMode(dshot_pin, OUTPUT);
}

void Dshot::set_speed_standard(dshot_standard_t standard)
{
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

void Dshot::send(int16_t command)
{
	// Error if not initialized
	if(!dshot_timings[0].first)
	{
		throw std::runtime_error("Dshot not initialized");
	}

	if(command > 2047)
	{
		command = 2047;
	}

	// Append telemetry request bit
	int16_t message = command << 1;

	// Append cyclic redundancy check (CRC)
	int8_t crc = (message ^ (message >> 4) ^ (message >> 8)) & 0x0F;

	message = message << 4 | crc;

	// printf("0x%X\n", message);

	// Send bits
	for(int8_t bit = 15; bit >= 0; bit--)
	{
		send_bit((message >> bit) & 1);
	}
}

void Dshot::throttle(int16_t throttle)
{
	// Max throttle is 1999
	if (throttle > 1999)
	{
		throttle = 1999;
	}
	else if (throttle < 0)
	{
		throttle = 0;
	}

	// Throttle ranges from 48-2047
	throttle += 48;

	send(throttle);
}

void Dshot::send_bit(bool value)
{
	// Time high
	digitalWrite(dshot_pin, 1);
	busy10ns(dshot_timings[value].first);

	// Time low
	digitalWrite(dshot_pin, 0);
	busy10ns(dshot_timings[value].second);
}

void Dshot::startup()
{
	for(int i = 0; i < 1000; i++)
    {
        busy10ns(100000);
        throttle(1999);
    }
    for(int i = 0; i < 1000; i++)
    {
        busy10ns(100000);
        throttle(0);
    }
}

void group_startup(std::vector<Dshot> motor_list)
{
	for(int i = 0; i < 1000; i++)
    {
        busy10ns(100000);
		for(int i = 0; i < motor_list.size(); i++)
		{
        	motor_list[i].throttle(1999);
		}
    }
    for(int i = 0; i < 1000; i++)
    {
        busy10ns(100000);
		for(int i = 0; i < motor_list.size(); i++)
		{
        	motor_list[i].throttle(0);
		}
    }
}
