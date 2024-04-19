# Autosteering bridge firmware

This firmware connects an ESP32 module to a Teensy 4.1 module over serial and handles the wireless network communication with the Autosteering application.

## Configuration

The ESP32 will start scanning for known networks on startup, and connect to the one with the strongest signal strength. If no known network is found in range, an access point network will be created.

To access the configuration web site, connect to the same network and go to the hostname address in a web browser, by default this is `http://autosteering.local/` or `http://autosteering/` depending on the browser. If you are connected to the access point network, the default IP is `192.168.4.1` in case the hostname address doesn't work.
