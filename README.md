# Instructions
## Configuration
### Clone and install WiringPi
https://github.com/WiringPi/WiringPi

Run the supplied build script

`./build`

### Configure Raspberry Pi
Run `sudo raspi-config` and set the following settings:
- System Options > Boot / Autologin > Console Autologin
- Interface Options > SSH > Yes
- Interface Options > SPI > Yes
- Interface Options > I2C > Yes

### Overclock
`sudo nano /boot/config.txt`

Add
> arm_freq=1000

> over_voltage=3

### Set I2C clock speed
`sudo nano /boot/config.txt`

Insert 
> dtparam=i2c1_baudrate=300000

This may need to be different (300k seems to be better for the Pi 4)

## Wiring
See `gpio readall` for pinout or refer to the following image.
SD slot is oriented at the top.
![](https://pi4j.com/1.2/images/j8header-zero-large.png)

## Remote Development via SSH
- Install Visual Studio Code
- Install the "Remote - SSH" extension
- Either: 
  - Physically connect the Raspberry Pi to the laptop with an ethernet cord
  - Connect the Raspberry Pi to the same WiFi network as the laptop
    - Mobile hotspots can work but connection may be inconsistent.
- Open a remote window and connect to host `pi@raspberrypi.local`
- Press "Continue" and type the password "pi"
- If connection fails, try deleting the SSH `known_hosts` file and reconnect.
  - On Windows, this file is located in `%USERPROFILE%\.ssh`

## Datasheets
[HC-SR04 Ultrasonic Ranging Module](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)

[MTi-1 Series IMU](https://mtidocs.xsens.com/mti-1-series)

[MTi Low-Level Documentation](https://www.xsens.com/hubfs/Downloads/Manuals/MT_Low-Level_Documentation.pdf)

[DShot](https://brushlesswhoop.com/dshot-and-bidirectional-dshot/)

[WiringPi](http://wiringpi.com/)

[libcamera](https://www.raspberrypi.com/documentation/computers/camera_software.html)

[Ultimate GPS on RPi](https://learn.adafruit.com/adafruit-ultimate-gps-on-the-raspberry-pi/introduction)
