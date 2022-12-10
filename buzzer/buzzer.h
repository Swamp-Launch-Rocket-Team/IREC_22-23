#ifndef BUZZER_H
#define BUZZER_H

#include <wiringPi.h>

void buzzer_init(int pin_number);

static void buzzer_length(int milliseconds);

void buzzer_long();

void buzzer_short();

#endif
