default: dshot_test

dshot_test: dshot.cpp dshot.h busynano.cpp dshot_test.cpp
	c++ -std=c++11 -g -O0 -o dshot_test dshot_test.cpp dshot.cpp busynano.cpp -lwiringPi

clean:
	rm -f dshot_test
