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

const int MOTOR_ENABLE_PIN = 17;

const int SENSOR_PERIOD_US = 10000; // 100 Hz, higher might intervene with GNSS messages, might
                                    // have to be lowered further when adding more sensors/data.
                                    // Sets how often sensor messages get sent from the hardware.

const int STEPPER_PERIOD_US = 20000; // 0.02 s

const int STEPPER_COMMAND_UPDATE_US = 1000000; // 1 s Disable stepper motor if no command has been received
                                               // within this time sine last command.

const int PIN_SPI_MISO1 = 19;
const int PIN_SPI_MOSI1 = 23;
const int PIN_SPI_SCK1 = 18;
const int PIN_SPI_SS1 = 5;

const int PIN_SPI_MISO2 = 12;
const int PIN_SPI_SCK2 = 14;
const int PIN_SPI_SS2 = 15;

#endif