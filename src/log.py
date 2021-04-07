import camera
import logging
import pyb

def log_data(sensors):
    try:
        for sensor in sensors.values():
            sensor.update()
            logging.info(str(sensor))

    except BaseException as err:
        logging.error("Failed to save data with error type {}!".format(err))
        raise
    return


def log_image(dir):
    try:
        file_name = camera.take_image(dir)
        logging.info("Successfully saved image to file {}.".format(file_name))
    except BaseException as err:
        logging.error("Failed to save image with error type {}!".format(err))
        raise
