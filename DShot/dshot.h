#ifndef DSHOT_H
#define DSHOT_H

#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <wiringPi.h>
#include <cstring>
#include <vector>
#include "busynano/busynano.h"

class Dshot {
	// Timings in nanoseconds.
	// Array index 1 represents logical 1. Array index 0 represents logical 0.
	// First value in pair is time high. Second value is time low.
	static std::pair<long, long> dshot_timings[2];

public:
	typedef enum 
	{
		DSHOT150,
		DSHOT300,
		DSHOT600,
		DSHOT1200,
	} dshot_standard_t;

	uint8_t dshot_pin;

	Dshot(uint8_t pin_number);

	static void set_speed_standard(dshot_standard_t standard);

	void send(uint16_t command);

	void throttle(uint16_t throttle);

	void send_bit(bool value);

	void startup();
};

void group_startup(std::vector<Dshot> motor_list);

#endif
