from picamera2 import *
import time

picam2 = Picamera2()

config = picam2.create_still_configuration(main={"size": (1920, 1080)})

picam2.configure(config)

picam2.shutter_speed = 1

picam2.start()

start = time.time()

index = 0

while time.time() - start < 60 * 10:
    picam2.capture_file('images/image' + str(index) + '.jpg')
    time.sleep(5)
    index = index + 1

picam2.close()
