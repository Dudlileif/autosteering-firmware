# Teensy firmware

This firmware connects a Teensy 4.1 module to following components:

- [Serial] [ESP32 wireless bridge](../esp32/autosteering_bridge/).
- [Serial] [Adafruit BNO085](https://www.adafruit.com/product/4754) inertial measurement unit (IMU) based on the [Ceva BNO08x](https://www.ceva-ip.com/product/bno-9-axis-imu/) in RVC mode.
- [Serial] [Ardusimple simpleRTK2B Micro](https://www.ardusimple.com/product/simplertk2b-micro/) GNSS receiver based on the [u-blox ZED-F9P](https://www.u-blox.com/en/product/zed-f9p-module).
- [SPI] [Trinamic TMC5160-BOB](https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/tmc5160-bob.html) stepper driver for controlling the steering wheel stepper motor [SM2563C20B41P](https://products.sanyodenki.com/en/sanmotion/stepping/f2/SM2563C20B41/).
- [ADC] Wheel angle sensor (WAS), a hall effect sensor measuring the angle of a steered wheel, typical car ride height/level sensor.

## Configuration

Configuration of the motor and WAS can be done through the web interface of the [ESP32 wireless bridge](../esp32/autosteering_bridge/).
