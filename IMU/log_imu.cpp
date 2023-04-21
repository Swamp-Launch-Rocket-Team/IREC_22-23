#include "imu.h"
#include <chrono>
#include <fstream>
#include <list>
#include <wiringPi.h>
#include <exception>
#include <signal.h>
#include <stdlib.h>

using namespace std;

class InterruptException : public std::exception
{
    public:
        InterruptException(int s) : S(s) {}
        int S;
};

void sig_to_exception(int s);

int main()
{
    // Turn off buzzer
    wiringPiSetup();
	pinMode(21, OUTPUT);
    digitalWrite(21, LOW);

    // Initialize all subsystems
    // Initialize IMU
    int file;
    int imu_address = 0x6B;
    file = imu_init(imu_address);

    imu_data_t state;

    auto start = chrono::high_resolution_clock::now();
    auto cur = chrono::high_resolution_clock::now();

    ofstream log_file;
    log_file.open("plog.csv");

    try
    {
        while (chrono::duration_cast<chrono::minutes>(cur - start).count() < 15)
        {
            cur = chrono::high_resolution_clock::now();

            state = imu_read_data();

            int delay = 500000;
            busy10ns(delay); // 5 ms delay

            if (state.pressure != 0)
            {
                log_file << chrono::duration_cast<chrono::microseconds>(cur - start).count() << "," << state.pressure << endl;
            }
            
            while (chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - start).count() < 9500);
        }
    }
    catch (InterruptException& e)
    {
        log_file << "End" << endl;
        log_file.close();
        return 1;
    }
    
    

    log_file.close();

    return 0;
}

void sig_to_exception(int s)
{
    throw InterruptException(s);
}
