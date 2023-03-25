default:
	c++ -std=c++11 -g -o main main.cpp IMU/imu.cpp controller/controller.cpp controller/PID.cpp controller/parse_cfg.cpp DShot/dshot.cpp DShot/busynano/busynano.cpp -lwiringPi -lconfig4cpp

clean:
	rm -f main
