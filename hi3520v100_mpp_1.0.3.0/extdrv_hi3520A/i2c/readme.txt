before insmod i2c.ko, you must modify multiplexing register:
himd 0x200F0198 1
himd 0x200F019C 1
