#include <iostream>
#include <string>
#include "dshot.h"
#include "busynano.h"

int main()
{
    dshot_init(8, DSHOT600);

    // unsigned short throttle = 0;
    // while(1)
    // {
    //     std::string s;
    //     std::getline(std::cin, s);
    //     try { throttle = stoi(s); } catch (std::invalid_argument) {}
    //     dshot_send(throttle);
    // }

    while(1)
    {
        busy10ns(500000);
        dshot_send(1999);
    }

    // while(1)
    // {
    //     send_bit(0);
    // }

    return 0;
}
