# Jake Stenger, 1/16/21
# Intended for PyBoard v1.1


import log, sensors
import logging
import time
from pyb import LED


colors = {"RED": 1, "GREEN": 2, "YELLOW":3, "BLUE": 4}
leds = {"red": LED(1), "green": LED(2), "yellow": LED(3), "blue": LED(4)}


def main():
    logging.basicConfig("test.log") # need to implement timestamp for log file name
    count = 0

    while True:
        if count % 60 == 0:
            try:
                log.log_data()
                # light = sensors.get_light_data()
                light = {'ch0': 0, 'ch1': 0}  # for testing
                if light.ch0 > 50 or light.ch1 > 50 or count % 3600 == 0:
                    log.log_image()
                    count = 0
            except:
                leds.red.on()
                raise
            else:
                leds.green.on()
            count += 1
        else:
            leds.blue.on()

        time.sleep_ms(500)
        for led in leds:
            led.off()
        time.sleep_ms(500)










