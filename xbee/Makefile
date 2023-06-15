default: xbee_test interpreter_test

xbee_test: xbee_test.cpp xbee_uart2.cpp xbee_uart2.h
	c++ -std=c++17 -Wall -g -o xbee_test xbee_test.cpp xbee_uart2.cpp

interpreter_test: interpreter_test.cpp xbee_interpreter.cpp xbee_interpreter.h xbee_uart2.cpp xbee_uart2.h ../controller/state.h ../IMU/imu.h
	c++ -std=c++17 -Wall -g -o interpreter_test interpreter_test.cpp xbee_interpreter.cpp xbee_uart2.cpp -lpthread

clean:
	rm -f xbee_test interpreter_test
