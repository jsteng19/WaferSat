import os
import sensor
import pyb



def init():
    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    sensor.skip_frames()

def take_image(dir):
    img = sensor.snapshot()

    img_name = "{}/{}.jpg".format(dir, pyb.millis())
    img.save(img_name)
    return img_name
