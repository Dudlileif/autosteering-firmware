#ifndef CONFIG_H
#define CONFIG_H

#include <ArduinoJson.h>

#define NETWORK_SERIAL Serial2 // RX:  7, TX:  8
#define GNSS_SERIAL Serial3    // RX: 15, TX: 14
#define IMU_SERIAL Serial5     // RX: 21, TX: 20

const unsigned long USB_BAUD = 921600;
const unsigned long DATA_BAUD = 921600;
const unsigned long GNSS_BAUD = 460800;
const unsigned long IMU_BAUD = 115200;

const int PRIORITY_MESSAGE_SIGNAL_PIN = 32;

const int LED_PIN = 13;

const int MOTOR_ENABLE_PIN = 2;

const int IMU_UPDATE_PERIOD_US = 10000; // 100 Hz, IMU in RVC mode outputs 100 Hz on serial.

const int SENSOR_PERIOD_US = 10000; // 100 Hz, higher might intervene with GNSS messages, might
                                    // have to be lowered further when adding more sensors/data.
                                    // Sets how often sensor messages get sent from the hardware.

const int STEPPER_PERIOD_US = 20000; // 0.02 s

const int STEPPER_COMMAND_UPDATE_US = 200000; // 0.2 s Disable stepper motor if no command has been received
                                              // within this time sine last command.

const int PIN_SPI_MISO1 = 39;
const int PIN_SPI_MOSI1 = 26;
const int PIN_SPI_SCK1 = 27;
const int PIN_SPI_SS1 = 38;

const int SENSOR_DATA_SIZE = JSON_OBJECT_SIZE(12);

#endif