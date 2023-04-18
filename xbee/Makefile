default: xbee_test

xbee_test: xbee_test.cpp xbee_uart2.cpp xbee_uart2.h
	c++ -std=c++11 -Wall -g -o xbee_test xbee_test.cpp xbee_uart2.cpp

clean:
	rm -f xbee_test
