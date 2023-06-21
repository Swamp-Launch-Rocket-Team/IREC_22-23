default:
	c++ -std=c++17 -g -o tuning  tuning.cpp IMU/imu.cpp controller/controller.cpp controller/PID.cpp controller/parse_cfg.cpp DShot/dshot.cpp DShot/busynano/busynano.cpp ultrasonic/ultrasonic.cpp -lwiringPi -lconfig4cpp -lpthread

clean:
	rm -f main
