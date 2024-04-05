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

const int TEENSY_BAUD = 921600;

const int TEENSY_SERIAL_TIMEOUT_MS = 1000; // 1.0 s

extern String teensyCrashReport;

extern bool teensyUnresponsive;

extern MotorConfig motorConfig;

extern unsigned long teensyUptimeMs;

extern unsigned long lastTeensyCommTime;

void sendMotorConfig();

bool getTeensyFirmwareVersion(bool debugPrint);

bool getTeensyCrashReport(bool debugPrint);

bool getTeensyUptime(bool debugPrint);

void rebootTeensy();

int readTeensySerial(uint8_t *buffer);

void checkIfTeensyIsResponding();

#endif