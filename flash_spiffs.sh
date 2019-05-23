#! /bin/bash

python $IDF_PATH/components/spiffs/spiffsgen.py 512000 ./spiffs_image/ ./build/storage.bin
python $IDF_PATH/components/esptool_py/esptool/esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x110000 ./build/storage.bin
