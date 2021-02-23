# https://docs.micropython.org/en/latest/library/machine.I2C.html
from machine import I2C
from LTR329ALS01 import *
from TMP100 import *

sensor_addresses = {'BME280': 0x77, 'TMP100_1': 0x48, 'TMP100_2': 0x49, 'BMX160': 0x68, 'LTR_329ALS_01': 0x29}


class Sensors:
    def __init__(self, bus=2):
        i2c = I2C(bus)          # create I2C peripheral at frequency of 400kHz
        self.tmp2 = TMP100(i2c, sensor_addresses['TMP100_2'])
        self.ltr = LTR329ALS01(i2c)

    # def get_humidity_data(self):  # stub
    #     pass
    #
    # def get_temp1_data(self):  # stub
    #     pass
    #     # if(i2c.is_ready())
    #
    # def get_temp2_data(self):  # stub
    #     tmp = TMP100(self.i2c, sensor_addresses['TMP100_2'])
    #
    # def get_orientation_data(self):  # stub
    #     pass
    #
    # def get_light_data(self):  # stub
    #     ltr = LTR329ALS01(self.i2c)
    #     return ltr.light()


