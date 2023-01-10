#include <iostream>
#include <wiringPi.h>
#include "ultrasonic.h"

int main()
{
    ultrasonic_init(8, 9);

    while(true)
    {
        float distance = ultrasonic_read();
        std::cout << "\r" << std::flush << distance << "     ";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
