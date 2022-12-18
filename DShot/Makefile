default: dshot_test

dshot_test: dshot.cpp dshot.h busynano/busynano.cpp dshot_test.cpp
	c++ -std=c++11 -g -o dshot_test dshot_test.cpp dshot.cpp busynano/busynano.cpp -lwiringPi

clean:
	rm -f dshot_test
