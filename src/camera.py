import os

def take_image(dir):
    #stub, only performs logging
    images = os.listdir(dir)
    if images:
        prev_img_name = max(images)
        prev_img_num = int(prev_img_name[3:])
    else:  # there are no images
        prev_img_num = -1
    img_name = dir + "/img" + str(prev_img_num + 1) + ".jpg"

    return img_name
