#include <iostream>
#include <wiringPi.h>
#include "ultrasonic.h"

int main()
{
    ultrasonic_init(8, 9);

    wiringPiSetup();
    pinMode(7, OUTPUT);

    while(true)
    {
        digitalWrite(7, HIGH);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        digitalWrite(7, LOW);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}
