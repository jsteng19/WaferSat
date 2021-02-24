### adapted from: https://forum.micropython.org/viewtopic.php?t=2814

import logging

class TMP100:

    # Registers
    T_REG = const(0x00)
    CONF_REG = const(0x01)

    # Configuration Register
    SHUTDOWN = const(0x01)
    RES_9bits = const(0x00)  # 9bits, 0.5C, 40ms
    RES_10bits = const(0x20)  # 10bits, 0.25C, 80ms
    RES_11bits = const(0x40)  # 11bits, 0.125C, 160ms
    RES_12bits = const(0x60)  # 12bits, 0.0625C, 320ms
    ONESHOT = const(0x80)

    def __init__(self, i2c, addr, res=RES_12bits):
        self.i2c = i2c
        self.addr = addr
        try:
            if addr not in i2c.scan():
                raise
            self.set_resolution(res)

        except:
            logging.error("Error initializing TMP on i2c address " + str(self.addr))

        else:
            logging.info("Successfully initialized TMP.")

    def set_resolution(self, res):
        buf = bytearray([res])
        self.i2c.writeto_mem(self.addr, CONF_REG, buf)

    def conv_temp(self, t):
        temp = ((t[0] << 8 | t[1]) >> 4)
        if temp & 0x800:  # sign bit set
            temp = -((temp ^ 0xfff) + 1)  # two's complement conversion
        return temp * 0.0625

    def read(self):
        raw = self.i2c.readfrom_mem(self.addr, T_REG, 2)
        tmp = self.conv_temp(raw)
        return tmp