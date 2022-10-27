from digi.xbee.devices import XBeeDevice
from digi.xbee.io import IOLine, IOMode
import time

device = XBeeDevice("COM4", 9600)
device.open()
device.set_sync_ops_timeout(10)

device.set_io_configuration(IOLine.DIO3_AD3, IOMode.ADC)

while(1):
    #moisture = device.get_adc_value(IOLine.DIO3_AD3)
    str_moisture = "M" + "1000" # Convert to string for transmission
    device.send_data_broadcast(str_moisture)
    print(str_moisture)
    time.sleep(0.125)