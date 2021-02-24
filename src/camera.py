import os
import sensor

def init():
    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    sensor.skip_frames()

def take_image(dir='images'):
    img = sensor.snapshot()

    images = os.listdir(dir)
    if images:
        prev_img_name = max(images)
        prev_img_num = int(prev_img_name[3:])
    else:  # there are no images
        prev_img_num = -1
    img_name = dir + "/img" + str(prev_img_num + 1) + ".jpg"
    img.save(img_name)

    return img_name
