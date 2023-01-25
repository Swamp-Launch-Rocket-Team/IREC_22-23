# Dylan Ogrodowski
# 1/24/2023
# https://www.digi.com/support/knowledge-base/an-introduction-to-spi-on-xbee-radios
# https://www.digi.com/resources/documentation/digidocs/pdfs/90002173.pdf "Select the SPI Port"
# Untested until SPI hardware connections are set up

import time
import spidev
import RPi.GPIO as GPIO

bus = 0 # SPI0, this is the only SPI option for RPi
cs = ??? # Manually driven CS pin, this is what is actually used

# Device is unused, CS is manually driven
device = 1 # Device is automatically driven CS pin, we do not use this in this household

# Enable SPI
spi = spidev.SpiDev()

# Open a connection to a specific bus and device (chip select pin)
GPIO.output(cs,True)
spi.open(bus, device)

# Set SPI speed and mode
spi.max_speed_hz = 500000
spi.mode = 0

# Clear display
msg = ['a']
GPIO.setup(cs, GPIO.OUT)

# TO DO: HOLD DOUT LOW, THEN RESET, THEN WAIT FOR ASSERT LINE

while 1:
    # Output message
    GPIO.output(cs,False)
    spi.xfer2(0x7E) # Start of frame delimiter
    spi.xfer2(length_MSB) # Length MSB
    spi.xfer2(length_LSB) # Length LSB
    spi.xfer2(msg)
    spi.xfer2(checksum)
    GPIO.output(cs,True)
    
    # THIS WILL NEED TO BE MODIFIED FOR BIDIRECTIONAL COMMUNICATION
    # ATTENTION PIN MUST BE CHECKED
    time.sleep(1)