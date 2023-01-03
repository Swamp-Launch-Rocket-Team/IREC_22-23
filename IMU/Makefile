default: imu_test

imu_test: imu.cpp imu.h imu_test.cpp
	c++ -std=c++11 -g -o imu_test imu_test.cpp imu.cpp

drift_test: imu.cpp imu.h imu_drift_test.cpp
	c++ -std=c++11 -g -o drift_test imu_drift_test.cpp imu.cpp

calibration_test: imu.cpp imu.h calibration_test.cpp
	c++ -std=c++11 -g -o calibration_test calibration_test.cpp imu.cpp

clean:
	rm -f imu_test calibration_test drift_test