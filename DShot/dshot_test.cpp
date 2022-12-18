#include <iostream>
#include <string>
#include "dshot.h"
#include "busynano/busynano.h"

int main(int argc, char* argv[])
{
    // Initialization
    dshot_init(8, DSHOT1200);
    std::cout << "Press enter to sent startup command" << std::endl;
    std::string s;
    std::getline(std::cin, s);
    for(int i = 0; i < 300; i++)
    {
        busy10ns(100000);
        dshot_throttle(1999);
    }
    for(int i = 0; i < 300; i++)
    {
        busy10ns(100000);
        dshot_throttle(0);
    }
    for(int i = 0; i < 300; i++)
    {
        busy10ns(100000);
        dshot_throttle(1999);
    }
    for(int i = 0; i < 300; i++)
    {
        busy10ns(100000);
        dshot_throttle(0);
    }
    std::cout << "Startup complete." << std::endl;
    int throttle;
    for(throttle = 700; throttle < 2200; throttle++)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(200000);
            dshot_throttle(throttle);
        }
    }
    for(; throttle > 500; throttle--)
    {
        for(int i = 0; i < 5; i++)
        {
            busy10ns(200000);
            dshot_throttle(throttle);
        }
    }

    return 0;
}
