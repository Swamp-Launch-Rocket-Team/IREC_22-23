#!/usr/bin/env python3

import serial
import time
from xbee import XBee
SERIAL_PORT = "COM3"
BAUD_RATE = 9600

# configure the xbee
ser = serial.Serial(SERIAL_PORT, baudrate=BAUD_RATE)
xbee = XBee(ser, escaped=False)

# handler for sending data to a receiving XBee device
def send_data(data):
    xbee.send("tx", dest_addr=b'\x00\x00', data=bytes("{}".format(data), 'utf-8'))

while True:
    try:
        send_data("Meow! :3")
        time.sleep(1)
    except KeyboardInterrupt:
        break

# clean up
xbee.halt()
ser.close()
