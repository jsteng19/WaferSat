import camera, sensors
import logging

IMG_FOLDER = "images"

def log_data():
    # stub
    try:
        data = "filler data"
        logging.info(data)
        pass
    # log data from sensors
    except BaseException as err:
        logging.error("Failed to save data with error type {}!".format(err))
        raise
    return

def log_image():
    # stub
    try:
        file_name = camera.take_image(IMG_FOLDER)
        logging.info("Successfully saved image to file {}.".format(file_name))
    except BaseException as err:
        logging.error("Failed to save image with error type {}!".format(err))
        raise
