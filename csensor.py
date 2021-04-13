from ltr import LTR
from tmp import TMP
from bme import BME

from machine import I2C

def init():
    sensor_addresses = {'BME280': 0x77, 'TMP100_0': 0x48, 'TMP100_1': 0x49, 'BMX160': 0x68, 'LTR_329ALS_01': 0x29}
    i2c = I2C()  # create I2C peripheral at frequency of 400kHz

    TMP(i2c=i2c, name='TMP100_1', addr=sensor_addresses['TMP100_1'])
    LTR(i2c=i2c, name='LTR_329ALS_01', addr=sensor_addresses['LTR_329ALS_01'])
    

class Sensor(object):
    most_recent = {} # a dict of the most recent sensor values
    units = [] # a list of acceptable units for this sensor
    name = "" # this sensors name
    sensors = {}  # static dict of all sensor-type objects
    
    def __init__(self, name):
        self.name = name
        Sensor.sensors[name] = self

    def update(self, *args) -> {"dtype": 1.234}:
        """
        Refresh the value by acquiring new data from the sensor, and then return
        it.
        """
        self.most_recent = {}
        return self.get(*args)

    def get(self, units=None) -> {"dtype": 1.234}:
        """
        Return the most recently acquired value.
        """
        return self.most_recent

    def __str__(self, *args):
        """
        Return a string with the most recent value and the name of the sensor.
        """
        return "{}: {}".format(self.name, self.get(*args))
