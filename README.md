# MQTT Receiver
This is an open source project aimed at creating a customizable MQTT firmware for the ESP32 with a captive portal Wi-Fi configuration interface for setup.

# Building
Requires a functioning [ESP-IDF](https://github.com/espressif/esp-idf) v3.2 environment for building and flashing the firmware. To flash the SPIFFS, please download the `spiffsgen.py` script from ESP-IDF's master branch [here](https://github.com/espressif/esp-idf/blob/master/components/spiffs/spiffsgen.py) and place it into the appropriate folder of your `$IDF_PATH`.

	make flash && ./flash_spiffs.sh

# License
© 2019 Dylan Weber

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
