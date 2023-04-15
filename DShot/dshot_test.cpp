#include <iostream>
#include <string>
#include<cstdlib>
#include "dshot.h"
#include "busynano/busynano.h"
#include "../IMU/imu.h"

int main(int argc, char* argv[])
{
    // Initialization
    std::vector<Dshot> motors;
    motors.emplace_back(28);
    motors.emplace_back(29);
    motors.emplace_back(26);
    motors.emplace_back(27);
    Dshot::set_speed_standard(Dshot::DSHOT600);
    std::cout << "Press enter to sent startup command" << std::endl;
    std::string s;
    std::getline(std::cin, s);
    group_startup(motors);
    std::cout << "Startup complete." << std::endl;
    busy10ns(1000000);
    int throttle;
    // while(true)
    // {
    //     busy10ns(2000);
    //     throttle = 50;
    //     motors[0].throttle(throttle);
    //     motors[1].throttle(throttle);
    //     motors[2].throttle(throttle);
    //     motors[3].throttle(throttle);
    // }

    int delay = 1000000;
    int inc = delay / 50000;
    for(throttle = 100; throttle < 500; throttle += inc)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(delay);
            motors[0].throttle(throttle);
            motors[1].throttle(0);
            motors[2].throttle(0);
            motors[3].throttle(0);
        }
    }
    for(; throttle > 100; throttle -= inc)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(delay);
            motors[0].throttle(throttle);
            motors[1].throttle(0);
            motors[2].throttle(0);
            motors[3].throttle(0);
        }
    }

    for(throttle = 100; throttle < 500; throttle += inc)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(delay);
            motors[0].throttle(0);
            motors[1].throttle(throttle);
            motors[2].throttle(0);
            motors[3].throttle(0);
        }
    }
    for(; throttle > 100; throttle -= inc)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(delay);
            motors[0].throttle(0);
            motors[1].throttle(throttle);
            motors[2].throttle(0);
            motors[3].throttle(0);
        }
    }

    for(throttle = 100; throttle < 500; throttle += inc)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(delay);
            motors[0].throttle(0);
            motors[1].throttle(0);
            motors[2].throttle(throttle);
            motors[3].throttle(0);
        }
    }
    for(; throttle > 100; throttle -= inc)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(delay);
            motors[0].throttle(0);
            motors[1].throttle(0);
            motors[2].throttle(throttle);
            motors[3].throttle(0);
        }
    }

    for(throttle = 100; throttle < 500; throttle += inc)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(delay);
            motors[0].throttle(0);
            motors[1].throttle(0);
            motors[2].throttle(0);
            motors[3].throttle(throttle);
        }
    }
    for(; throttle > 100; throttle -= inc)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(delay);
            motors[0].throttle(0);
            motors[1].throttle(0);
            motors[2].throttle(0);
            motors[3].throttle(throttle);
        }
    }

    for(throttle = 100; throttle < 500; throttle += inc)
    {
        for(int i = 0; i < 5; i++)
        {
            if (i == 3)
            {
                busy10ns(5 * delay);
            }
            else
            {
                busy10ns(delay);
            }
            motors[0].throttle(throttle);
            motors[1].throttle(throttle);
            motors[2].throttle(throttle);
            motors[3].throttle(throttle);
        }
    }
    for(; throttle > 100; throttle -= inc)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(delay);
            motors[0].throttle(throttle);
            motors[1].throttle(throttle);
            motors[2].throttle(throttle);
            motors[3].throttle(throttle);
        }
    }

    // int file;
    // int imu_address = 0x6B;
    // file = imu_init(imu_address);
    // throttle = 500;
    // while(true)
    // {
    //     imu_data_t imu_data = imu_read_data();
    //     cout << imu_data.heading.x << endl;
    //     // int delay = (rand() % 50) * 10000;
    //     int delay = 1000000;
    //     busy10ns(delay);
    //     motors[0].throttle(throttle);
    //     motors[1].throttle(throttle);
    //     motors[2].throttle(throttle);
    //     motors[3].throttle(throttle);
    // }

    return 0;
}
