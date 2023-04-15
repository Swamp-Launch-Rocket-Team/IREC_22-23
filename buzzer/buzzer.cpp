#include "buzzer.h"
#define PERIOD 170

static int buzzer_pin;

void buzzer_init(int pin_number)
{
    // Set up output pin
	wiringPiSetup();
	buzzer_pin = pin_number;
	pinMode(buzzer_pin, OUTPUT);
}

static void buzzer_length(int milliseconds)
{
    for(int i = 0; i < milliseconds; i++)
    {
        digitalWrite(buzzer_pin, 1);
        delayMicroseconds(PERIOD);
        digitalWrite(buzzer_pin, 0);
        delayMicroseconds(PERIOD);
    }
}

void buzzer_long()
{
    for(int i = 0; i < 3; i++)
    {
        buzzer_length(1000);
        delay(1000);
    }
}

void buzzer_short()
{
    for(int i = 0; i < 3; i++)
    {
        buzzer_length(500);
        delay(500);
    }
}
