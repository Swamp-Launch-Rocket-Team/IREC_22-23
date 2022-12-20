#include <iostream>
#include <errno.h>
#include <wiringPiI2C.h>

using namespace std;

int main()
{
    int fd, result;

    // Initialize the interface by giving it an external device ID.
    // It returns a standard file descriptor.
    fd = wiringPiI2CSetup(0x6b);

    cout << "Init result: "<< fd << endl;

    result = wiringPiI2CWrite(fd, 0x01);
    if(result == -1)
    {
        cout << "Error.  Errno is: " << errno << endl;
        return result;
    }

    for(int i = 0; i < 8; i++)
    {
        int read = wiringPiI2CRead(fd);
        cout << "Read result: 0x" << hex << read << endl;
    }

    return 0;
}
