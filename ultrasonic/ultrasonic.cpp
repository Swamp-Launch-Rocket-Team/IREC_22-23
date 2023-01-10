#include "ultrasonic.h"

static void ultrasonic_write(float value);
static void sensor_read(int trigger_pin, int echo_pin);

// Thread-safe buffer
static struct Buffer
{
    std::mutex mutex;
    float value = -1;
} buffer;

// Initializes the ultrasonic sensor and starts the ultrasonic writing thread
void ultrasonic_init(int trigger_pin, int echo_pin)
{
    static bool initialized = false;
    if(initialized)
        return;
    initialized = true;

    // Set up wiring pi
    wiringPiSetup();
	pinMode(trigger_pin, OUTPUT);
    pinMode(echo_pin, INPUT);

    // Start thread
    std::thread sensor_thread(sensor_read, trigger_pin, echo_pin);
    sensor_thread.detach();
}

// Thread-safe function to read form the buffer
float ultrasonic_read()
{
    buffer.mutex.lock();
    float value = buffer.value;
    buffer.mutex.unlock();
    return value;
}

// Thread-safe function to write to the buffer
static void ultrasonic_write(float value)
{
    buffer.mutex.lock();
    buffer.value = value;
    buffer.mutex.unlock();
}

static void sensor_read(int trigger_pin, int echo_pin)
{
    float distance;
    while(true)
    {
        // Send a 10 microsecond pulse to the trigger pin
        digitalWrite(trigger_pin, HIGH);
        std::this_thread::sleep_for(std::chrono::microseconds(20));
        digitalWrite(trigger_pin, LOW);

        // Wait for the echo pin to go high (indicating the start of the echo pulse)
        while(digitalRead(echo_pin) == LOW);

        // Record the start time of the echo pulse
        auto start_time = std::chrono::high_resolution_clock::now();

        // Wait for the echo pin to go low (indicating the end of the echo pulse)
        while(digitalRead(echo_pin) == HIGH);

        // Record the end time of the echo pulse and calculate the distance
        auto end_time = std::chrono::high_resolution_clock::now();
        distance = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() / 58;

        // Write the distance to the buffer
        ultrasonic_write(distance);

        // Sleep for 100 milliseconds
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}
