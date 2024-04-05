// Copyright (C) 2024 Gaute Hagen
//
// This file is part of Autosteering Firmware.
//
// Autosteering Firmware is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Autosteering Firmware is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Autosteering Firmware.  If not, see <https://www.gnu.org/licenses/>.

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

const int SENSOR_PERIOD_US = 10000; // 100 Hz, higher might intervene with GNSS messages, might
                                    // have to be lowered further when adding more sensors/data.
                                    // Sets how often sensor messages get sent from the hardware.

const int STEPPER_PERIOD_US = 20000; // 0.02 s

const int STEPPER_COMMAND_UPDATE_US = 1000000; // 1 s Disable stepper motor if no command has been received
                                               // within this time sine last command.

const int PIN_SPI_MISO1 = 39;
const int PIN_SPI_MOSI1 = 26;
const int PIN_SPI_SCK1 = 27;
const int PIN_SPI_SS1 = 38;

const int PIN_WAS = 41;

#endif