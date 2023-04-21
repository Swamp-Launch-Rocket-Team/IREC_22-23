#include "buzzer.h"
#define PERIOD 500000

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
    digitalWrite(buzzer_pin, 1);
    delayMicroseconds(milliseconds * 1000);
    digitalWrite(buzzer_pin, 0);
}

void buzzer_long()
{
    for(int i = 0; i < 3; i++)
    {
        buzzer_length(1000);
        delay(250);
    }
}

void buzzer_short()
{
    for(int i = 0; i < 3; i++)
    {
        buzzer_length(500);
        delay(250);
    }
}
