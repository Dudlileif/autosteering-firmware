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

#ifndef TEENSYCOMMS_H
#define TEENSYCOMMS_H

#include <MotorConfig.h>
#include <elapsedMillis.h>

const int TEENSY_BAUD = 921600;

const int TEENSY_SERIAL_TIMEOUT_MS = 1000; // 1.0 s

extern String teensyCrashReport;

extern bool teensyUnresponsive;

extern MotorConfig motorConfig;

extern unsigned long teensyUptimeMs;

extern elapsedMillis lastTeensyCommElapsedTime;

struct TeensyHardwareState
{
    uint16_t wasReading;
    uint16_t wasTarget;
    bool motorEnabled;
    bool motorStalled;
    float rpm;
    uint16_t stallguardResult;
    uint8_t currentScale;
    float yaw;
    float pitch;
    float roll;
    float accX;
    float accY;
    float accZ;
};

extern String gnssNmeaGns;
extern String gnssNmeaGst;
extern String gnssNmeaVtg;

extern TeensyHardwareState teensyHardwareState;

void sendMotorConfig();

bool getTeensyFirmwareVersion(bool debugPrint);

bool getTeensyCrashReport(bool debugPrint);

bool getTeensyUptime(bool debugPrint);

void rebootTeensy();

void readTeensySerial();

void checkIfTeensyIsResponding();

void parseTeensyData(char byte);
#endif