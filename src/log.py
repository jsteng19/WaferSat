import camera, i2c_sensors
import logging

def log_data():
    # stub
    try:
        data = "filler data"
        logging.info(data)
    # log data from sensors
    except BaseException as err:
        logging.error("Failed to save data with error type {}!".format(err))
        raise
    return

def log_image():
    # stub
    try:
        file_name = camera.take_image()
        logging.info("Successfully saved image to file {}.".format(file_name))
    except BaseException as err:
        logging.error("Failed to save image with error type {}!".format(err))
        raise
