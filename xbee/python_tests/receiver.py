#!/usr/bin/env python3

import serial, time
from xbee import XBee

SERIAL_PORT = "/dev/serial0"
BAUD_RATE = 9600

# handler for whenever data is received from transmitters - operates asynchronously
def receive_data(data):
    print("Received data: {}".format(data))

    print("Packet: {}".format(data))
    print("Data: {}".format(data['rf_data']))

# configure the xbee and enable asynchronous mode
ser = serial.Serial(SERIAL_PORT, baudrate=BAUD_RATE)
xbee = XBee(ser, callback=receive_data, escaped=False)

while True:
    try:
        # operate in async mode where all messages will go to handler
        time.sleep(0.001)
    except KeyboardInterrupt:
        break

# clean up
xbee.halt()
ser.close()
