# Autosteering firmware

This repository contains the firmware source code for the hardware
supplementing the [Autosteering](https://github.com/Dudlileif/autosteering) application. The hardware primarily consists of an ESP32 module and a Teensy 4.1 module for controlling and monitoring the steering and positioning of a vehicle.

- [Teensy41](teensy/) contains firmware for using a [Teensy 4.1](https://www.pjrc.com/store/teensy41.html) to read vehicle steering status, GNSS and IMU positioning and control a steering wheel motor.

- [Autosteering bridge](esp32/autosteering_bridge/) contains firmware for using an [ESP32](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/hw-reference/esp32/get-started-devkitc.html) as a WiFi bridge between the Teensy and the Autosteering application.

- [Base station relay](esp32/base_station_relay/) contains firmware for using an [ESP32](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/hw-reference/esp32/get-started-devkitc.html) as a WiFi relay between a GNSS receiver (u-blox ZED-F9P or similar) and another device running an NTRIP caster or similar. This can be useful when the GNSS receiver can't be directly connected to the caster device.
