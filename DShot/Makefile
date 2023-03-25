default: dshot_test

dshot_test: dshot.cpp dshot.h busynano/busynano.cpp dshot_test.cpp ../IMU/imu.cpp ../IMU/imu.h
	c++ -std=c++11 -g -o dshot_test dshot_test.cpp dshot.cpp busynano/busynano.cpp ../IMU/imu.cpp -lwiringPi

clean:
	rm -f dshot_test
