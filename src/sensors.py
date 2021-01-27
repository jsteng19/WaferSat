# https://docs.micropython.org/en/latest/library/machine.I2C.html
from machine import I2C

sensor_addresses = {'BME280': 77, 'TMP100_1': 48, 'TMP100_2': 49, 'BMX160': 68, 'LTR_329ALS_01': 29}


class Sensors:
    def __init__(self):
        self.i2c = I2C(freq=400000)          # create I2C peripheral at frequency of 400kHz

    def get_humidity_data(self):  # stub
        return self.i2c.readfrom('BME280')

    def get_temp1_data(self):  # stub
        return self.i2c.readfrom('TMP100-1')

    def get_temp2_data(self):  # stub
        return self.i2c.readfrom(sensor_addresses['TMP100_2'])

    def get_orientation_data(self):  # stub
        return self.i2c.readfrom(sensor_addresses['BMX160'])

    def get_light_data(self):  # stub
        return self.i2c.readfrom(sensor_addresses['LTR-329ALS_01'])


