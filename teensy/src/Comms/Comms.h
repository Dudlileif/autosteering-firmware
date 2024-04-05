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

#ifndef COMMS_H
#define COMMS_H

#include <Arduino.h>
#include <MultiStream.h>

extern MultiStream usbAndNetworkSerial;

extern boolean enableUSBSerial;

extern DMAMEM byte networkSerialReadBuffer[1024];
extern DMAMEM byte networkSerialWriteBuffer[256];

extern char serialMessage[1024];
extern uint16_t serialMessageLength;
extern bool serialMessageFinished;
extern bool serialMessageIsRTCM;

extern char networkMessage[1024];
extern uint16_t networkMessageLength;
extern bool networkMessageFinished;
extern bool networkMessageIsRTCM;

extern char gnssMessage[128];
extern uint8_t gnssMessageLength;
extern bool gnssSerialFinished;

void commsInit();

void handlePriorityMessage();

void sendToProgram(char *, int);

void sendSensorData();

void receiveGNSSData();

void receiveNetworkData();

void receiveUSBSerialData();

#endif