#include <iostream>
#include "buzzer.h"

int main()
{
    buzzer_init(21); // Check pins
    std::cout << "Playing slow beeps." << std::endl;
    buzzer_long();
    std::cout << "Playing fast beeps." << std::endl;
    buzzer_short();
}
