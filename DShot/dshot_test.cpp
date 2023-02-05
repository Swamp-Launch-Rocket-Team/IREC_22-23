#include <iostream>
#include <string>
#include "dshot.h"
#include "busynano/busynano.h"

int main(int argc, char* argv[])
{
    // Initialization
    std::vector<Dshot> motors;
    motors.emplace_back(0);
    motors.emplace_back(1);
    motors.emplace_back(2);
    motors.emplace_back(3);
    Dshot::set_speed_standard(Dshot::DSHOT600);
    std::cout << "Press enter to sent startup command" << std::endl;
    std::string s;
    std::getline(std::cin, s);
    group_startup(motors);
    std::cout << "Startup complete." << std::endl;
    busy10ns(10000000);
    int throttle;
    for(throttle = 700; throttle < 2200; throttle++)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(2000);
            motors[0].throttle(throttle);
            motors[1].throttle(0);
            motors[2].throttle(0);
            motors[3].throttle(0);
        }
    }
    for(; throttle > 500; throttle--)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(2000);
            motors[0].throttle(throttle);
            motors[1].throttle(0);
            motors[2].throttle(0);
            motors[3].throttle(0);
        }
    }

    for(throttle = 700; throttle < 2200; throttle++)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(2000);
            motors[0].throttle(0);
            motors[1].throttle(throttle);
            motors[2].throttle(0);
            motors[3].throttle(0);
        }
    }
    for(; throttle > 500; throttle--)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(2000);
            motors[0].throttle(0);
            motors[1].throttle(throttle);
            motors[2].throttle(0);
            motors[3].throttle(0);
        }
    }

    for(throttle = 700; throttle < 2200; throttle++)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(2000);
            motors[0].throttle(0);
            motors[1].throttle(0);
            motors[2].throttle(throttle);
            motors[3].throttle(0);
        }
    }
    for(; throttle > 500; throttle--)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(2000);
            motors[0].throttle(0);
            motors[1].throttle(0);
            motors[2].throttle(throttle);
            motors[3].throttle(0);
        }
    }

    for(throttle = 700; throttle < 2200; throttle++)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(2000);
            motors[0].throttle(0);
            motors[1].throttle(0);
            motors[2].throttle(0);
            motors[3].throttle(throttle);
        }
    }
    for(; throttle > 500; throttle--)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(2000);
            motors[0].throttle(0);
            motors[1].throttle(0);
            motors[2].throttle(0);
            motors[3].throttle(throttle);
        }
    }

    return 0;
}
