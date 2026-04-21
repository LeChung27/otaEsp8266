esptool.py -p /dev/ttyUSB0 --baud 921600 write_flash -fs 4MB -fm qio -ff 40m \
	0x0 ../../bootloader/firmware/rboot.bin \
	0x1000 ../../bootloader/firmware_prebuilt/blank_config.bin \
	0x82000 ./firmware/blink.bin
gtkterm -c ch340


 #esptool.py -p /dev/ttyUSB0 --baud 921600 read_flash 0xE2000 0x1000 ./spi_testE2000.bin
