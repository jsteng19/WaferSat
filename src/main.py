# Jake Stenger, 1/16/21
# For OpenMV board


import log, logging
import csensor
import time
import camera
from pyb import LED
import os

from LTR329ALS01 import LTR329ALS01
from TMP100 import TMP100
from machine import I2C

### init ###

LEDS = {"red": LED(1), "green": LED(2), "blue": LED(3)}
#INTERVAL = const(60)
INTERVAL = const(1) # increased freq for testing

sensor_addresses = {'BME280': 0x77, 'TMP100_1': 0x48, 'TMP100_2': 0x49, 'BMX160': 0x68, 'LTR_329ALS_01': 0x29}

files = os.listdir()
i = 0
while "run{}".format(i) in files:
    i += 1

dname = "run{}".format(i)
os.mkdir(dname)
fname = dname + "/out.log"
log_file = open(fname, 'a')
logging.basicConfig(stream=log_file)

sensors = {}

def init_sensors(bus=2):
    i2c = I2C(bus)  # create I2C peripheral at frequency of 400kHz
    sensors['tmp2'] = TMP100(i2c=i2c, name='tmp2', addr=sensor_addresses['TMP100_2'])
    sensors['ltr'] = LTR329ALS01(i2c=i2c)

init_sensors()
camera.init()

def main_loop():

    count = 0
    while True:
        if count % INTERVAL == 0:
            try:
                log.log_data(sensors)
                light = sensors['ltr'].get()
                print(light)
                if light["ch0"] > 50 or light["ch1"] > 50 or count % 3600 == 0:
                    log.log_image(dname)
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
