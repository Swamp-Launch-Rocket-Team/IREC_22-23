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

device = XBeeDevice("COM4", 9600)
device.open()
device.set_sync_ops_timeout(10)

print('IREC 2023 ground station sender terminal')
print('Press \'r\' to initiate release approval')

transmit_message = 'rel'

while(1):

    # Check for any new data and print to serial monitor
    xbee_message = device.read_data() # Read the messgae
    if xbee_message is not None: # Message will return None if nothing is received
        print(xbee_message.data.decode('utf8')) # Decode the message before printing to the serial monitor
        
    try:  # used try so that if user pressed other than the given key error will not be shown
        if keyboard.is_pressed('r'):  # if key 'r' is pressed
            print('Initializing release...')
            print('Press \'c\' to confirm release approval or \'x\' to cancel')
            confirm = False
            cancel = False
            while(not keyboard.is_pressed('c') and not keyboard.is_pressed('x')): # Wait for confirmation or cancellation
                pass # Idle
            
            if keyboard.is_pressed('x'):
                print('Cancelled release approval. Press \'r\' to reinitiate release approval')
                time.sleep(0.125)
                continue
            elif keyboard.is_pressed('c'):
                #device.send_data_broadcast(transmit_message)
                print("Release command sent")
                time.sleep(0.125)
                print('Press \'r\' to initiate release approval')
                
    except:
        break  # if user pressed a key other than the given key the loop will break