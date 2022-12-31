default: ultrasonic_test

ultrasonic_test: ultrasonic.cpp ultrasonic.h ultrasonic_test.cpp
	c++ -std=c++11 -Wall -g -o ultrasonic_test ultrasonic_test.cpp ultrasonic.cpp -lwiringPi -lpthread

clean:
	rm -f ultrasonic_test
