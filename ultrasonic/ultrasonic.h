#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <wiringPi.h>
#include <thread>
#include <mutex>

// Initializes the ultrasonic sensor and starts the ultrasonic writing thread
void ultrasonic_init(int trigger_pin, int echo_pin);

// Thread-safe function to read form the buffer
float ultrasonic_read();

#endif
