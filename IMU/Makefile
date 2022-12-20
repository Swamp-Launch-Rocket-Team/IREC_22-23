default: imu_test

imu_test: imu.cpp imu.h imu_test.cpp
	c++ -std=c++11 -g -o imu_test imu_test.cpp imu.cpp -lwiringPi

clean:
	rm -f imu_test
