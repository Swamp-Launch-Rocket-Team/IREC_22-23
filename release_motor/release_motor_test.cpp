#include <wiringPi.h>
#include <chrono>

using namespace std;

int main()
{
    wiringPiSetup();
    pinMode(5, OUTPUT);
    pinMode(21, OUTPUT);
    digitalWrite(21, LOW);

    auto start = chrono::high_resolution_clock::now();
    digitalWrite(5, HIGH);


    while (true)
    {
        if (chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count() > 1000)
        {
            digitalWrite(5, LOW);
            break;
        }
    }
}
