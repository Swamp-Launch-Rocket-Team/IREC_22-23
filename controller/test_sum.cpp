#include <vector>
#include <iostream>
#include <chrono>
#include <random>

using namespace std;

int main()
{
    srand (time(NULL));
    vector<float> arr;
    arr.resize(2001,0);
    for(int i = 0; i < arr.size(); i++)
    {
        arr[i] = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/2000));
    }

    auto start = chrono::high_resolution_clock::now();

    float sum = 0;
    for(int x : arr)
    {
        sum += x;
    }
    auto end = chrono::high_resolution_clock::now();

    cout << "Sum: " << sum << endl;
    cout << "Time: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << endl;
}