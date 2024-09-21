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

#ifndef SENSORS_H
#define SENSORS_H

#include <ArduinoJson.h>

#include "Config/Config.h"
#include "Adafruit_BNO08x_RVC.h"

extern elapsedMicros sensorPrevUpdateElapsedTime;

extern Adafruit_BNO08x_RVC imuRVC;

extern BNO08x_RVC_Data currentImuReading;
extern BNO08x_RVC_Data prevImuReading;

extern uint16_t wasReading;

float roundToNumberOfDecimals(float, int);

void imuInit();

void wasInit();

void updateImuReading();

JsonDocument getSensorData();

#endif
