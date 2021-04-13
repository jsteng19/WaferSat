import csensor, log, camera
import logging, time, pyb, os


RED, GREEN, BLUE = const(1), const(2), const(3)  # LED INDICES

files = os.listdir()
i = 0
while "run{}".format(i) in files:
    i += 1
dname = "run{}".format(i)
os.mkdir(dname)
fname = dname + "/out.log"
log_file = open(fname, 'a')
logging.basicConfig(stream=log_file)

camera.init()
csensor.init()

def sched_capture(t):
    global flag
    flag = 1

clock = time.clock()
tim = pyb.Timer(4)
tim.init(freq=2)
tim.callback(sched_capture)

DATA_INTERVAL = const(60)
IMAGE_INTERVAL = const(3600)

count = 0

def main():
    while True:
        if not flag:
            continue

        global flag
        flag = 0
        count += 1
        if count % 2 == 0:
            if count % 2 * DATA_INTERVAL == 0:
                try:
                    log.log_data(sensors)
                    light = sensors['ltr'].get()
                    if light["ch0"] > 50 or light["ch1"] > 50 or count % 2 * IMAGE_INTERVAL == 0:
                        log.log_image(dname)
                except:
                    pyb.LEDS(RED).on()
                else:
                    pyb.LEDS(GREEN).on()
            else:
                pyb.LEDS(BLUE).on()
        else:
            for led in (RED, GREEN, BLUE):
                pyb.LEDS(led).off()


try:
    main()
finally:
    log_file.close()
