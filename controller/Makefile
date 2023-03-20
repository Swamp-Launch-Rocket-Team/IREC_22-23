default: controller_test

controller_test: parse_cfg.cpp parse_cfg.h PID.cpp PID.h controller.cpp controller.h controller_test.cpp ../IMU/imu.cpp ../IMU/imu.h ../DShot/dshot.cpp ../DShot/dshot.h ../DShot/busynano/busynano.cpp ../DShot/busynano/busynano.h state.h
	c++ -std=c++11 -g -o controller_test controller_test.cpp parse_cfg.cpp PID.cpp controller.cpp ../IMU/imu.cpp ../DShot/dshot.cpp ../DShot/busynano/busynano.cpp -lconfig4cpp -lwiringPi

clean:
	rm -f controller_test