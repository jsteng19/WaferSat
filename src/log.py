import camera, i2c_sensors
import logging

def log_data(sensors):
    try:
        tmp2 = sensors.tmp2.read()
        logging.info("Successfully read TMP2 therm data: " + str(tmp2))

        light = sensors.ltr.read()
        logging.info("Successfully read LTR light data: " + str(light))
    # log data from sensors
    except BaseException as err:
        logging.error("Failed to save data with error type {}!".format(err))
        raise
    return


def log_image():
    try:
        file_name = camera.take_image()
        logging.info("Successfully saved image to file {}.".format(file_name))
    except BaseException as err:
        logging.error("Failed to save image with error type {}!".format(err))
        raise
