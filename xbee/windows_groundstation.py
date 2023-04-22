import serial
import time
from datetime import datetime

# current dateTime
now = datetime.now()

# convert to string
date_time_str = now.strftime("%Y-%m-%d %H %M %S")

serialPort = serial.Serial(
    port="COM3", baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
)
serialString = ""  # Used to hold data coming over UART

f = open(f"{date_time_str}.txt", "w")

while 1:
    try:
        # Read data out of the buffer until a carraige return / new line is found
        serialString = serialPort.readline()

        # Print the contents of the serial data
        try:
            f.write(serialString.decode("Ascii"))
            print(serialString.decode("Ascii"))
        except:
            pass
        time.sleep(0.1)
    except KeyboardInterrupt:
        break

# clean up
f.close()
serialPort.close()
