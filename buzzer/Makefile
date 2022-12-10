default: buzzer_test

buzzer_test: buzzer.cpp buzzer.h buzzer_test.cpp
	c++ -std=c++11 -g -O0 -o buzzer_test buzzer_test.cpp buzzer.cpp -lwiringPi

clean:
	rm -f buzzer_test
