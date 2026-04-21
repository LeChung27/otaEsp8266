esptool.py -p /dev/ttyUSB0 --baud 921600 write_flash -fs 4MB -fm qio -ff 40m \
	0x2000 ./firmware/blink2.bin
