import time
import serial
import RPi.GPIO as GPIO

ser = serial.Serial
(
    port='/dev/ttyS0',
    baudrate = 9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1            
 )

while 1:
    ser.write(str.encode('Write counter: %d \n'%(counter)))
    time.sleep(1)
    counter += 1