import camera

def log_data():
    # stub
    try:
        pass
    # log data from sensors
    except BaseException as err:
        log_error("Failed to save data with error type {}!".format(err))
        raise
    return


def get_light_data():
    # stub
    return {"ch0": 0, "ch1": 0}


def log_image():
    # stub
    try:
        camera.take_image()
    except BaseException as err:
        log_error("Failed to save image with error type {}!".format(err))
        raise

def log_error():
    #stub
    return