import serial
import time

serialPort = serial.Serial(
    port="COM3", baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
)
serialString = ""  # Used to hold data coming over UART
i = 0
while 1:
    # Send data
    serialPort.write(b"Ground Station %d\r\n" % i)

    # Wait until there is data waiting in the serial buffer
    # if serialPort.in_waiting > 0:

    # Read data out of the buffer until a carraige return / new line is found
    serialString = serialPort.readline()

    # Print the contents of the serial data
    try:
        print(serialString.decode("Ascii"))
    except:
        pass
    i += 1
    time.sleep(0.1)
