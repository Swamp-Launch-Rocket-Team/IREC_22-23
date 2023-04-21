#include <stdlib.h>
#include <iostream>

using namespace std;

int main()
{
    string script = "python camera.py &";
    system(script.c_str());

    cout << "Finished" << endl;

    return 0;
}