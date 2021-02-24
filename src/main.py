# Jake Stenger, 1/16/21
# For OpenMV board


import log, logging
import i2c_sensors
import time
import camera
from pyb import LED

### init ###

LEDS = {"red": LED(1), "green": LED(2), "blue": LED(3)}
#INTERVAL = const(60)
INTERVAL = const(5)

log_file = open('test.log', 'a')
logging.basicConfig(stream = log_file) # need to implement timestamp for log file name

sensors = i2c_sensors.Sensors()
camera.init()

def main_loop():

    count = 0
    while True:
        if count % INTERVAL == 0:
            try:
                log.log_data()
                light = sensors.ltr.read()
                print(light)
                if light[0] > 50 or light[1] > 50 or count % 3600 == 0:
                    log.log_image()
                    count = 0
            except:
                LEDS['red'].on()
                raise
            else:
                LEDS['green'].on()
        else:
            LEDS['blue'].on()

        count += 1

        time.sleep_ms(500)
        for led in LEDS.values():
            led.off()
        time.sleep_ms(500)

try:
    main_loop()
finally:
    log_file.close()