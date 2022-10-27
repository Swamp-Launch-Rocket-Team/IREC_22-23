from digi.xbee.devices import XBeeDevice

device = XBeeDevice("COM3", 9600) # Set the COM port and baud rate
device.open() # Open the COM port

while (1):
    xbee_message = device.read_data() # Read the messgae
    if xbee_message is not None: # Message will return None if nothing is received
        print(xbee_message.data.decode('utf8')) # Decode the message before printing to the serial monitor
