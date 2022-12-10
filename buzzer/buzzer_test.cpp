#include <iostream>
#include "buzzer.h"

int main()
{
    buzzer_init(8);
    std::cout << "Playing slow beeps." << std::endl;
    buzzer_long();
    std::cout << "Playing fast beeps." << std::endl;
    buzzer_short();
}
