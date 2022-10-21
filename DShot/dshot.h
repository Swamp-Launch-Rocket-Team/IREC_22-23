#ifndef DSHOT_H
#define DSHOT_H

#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <wiringPi.h>
#include <cstring>
#include "busynano.h"

typedef enum 
{
	DSHOT150,
	DSHOT300,
	DSHOT600,
	DSHOT1200,
} dshot_standard_t;

void dshot_init(uint8_t pin_number, dshot_standard_t standard);

void dshot_send(uint16_t throttle);

void send_bit(bool value);

#endif
