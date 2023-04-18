# IREC 2023 Ground Station
# Dylan Ogrodowski
# 3/1/2023

# Required setup installs:
# Python 3
# pip install keyboard
# pip install digi-xbee

from digi.xbee.devices import XBeeDevice
from digi.xbee.io import IOLine, IOMode
import time
import keyboard

device = XBeeDevice("/dev/serial0", 9600)
device.open()
device.set_sync_ops_timeout(3)

transmit_message = 'Pi'

while(1):
    # Check for any new data and print to serial monitor
    xbee_message = device.read_data() # Read the messgae
    if xbee_message is not None: # Message will return None if nothing is received
        print(xbee_message.data.decode('utf8')) # Decode the message before printing to the serial monitor
    device.send_data_broadcast(transmit_message)
    time.sleep(1)

