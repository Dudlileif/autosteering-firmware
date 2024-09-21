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

#ifndef _MAIN_CPP
#define _MAIN_CPP

#include <main.h>
#include <Network.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <OTAUpdate.h>
#include "TeensyComms/TeensyComms.h"
#include "TeensyOTAUpdateAdditions/TeensyOTAUpdateAdditions.h"
#include "TeensyWebServerAdditions/TeensyWebServerAddtions.h"

void setup()
{
    pinMode(PRIORITY_MESSAGE_SIGNAL_PIN, OUTPUT);
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, HIGH);
    mainSetup();
    addTeensyCallbacksToWebServer();

    TEENSY_SERIAL.setRxBufferSize(1024);
    TEENSY_SERIAL.begin(TEENSY_BAUD, SERIAL_8N1, RXD2, TXD2);
    motorConfig.loadFromFile(&LittleFS);

    Serial.println("Saved Motor config json:");
    motorConfig.printToStreamPretty(&Serial);

    delay(100);

    sendMotorConfig();

    getTeensyCrashReport(true);

    getTeensyFirmwareVersion(true);
}
    
void loop()
{
    if (doUpdate && updateFileName.endsWith(".hex"))
    {
        attemptTeensyUpdate();
    }
    if (mainLoop())
    {
        readTeensySerial();

        // UDP maxes out at 1460
        char udpPacketBuffer[1460];

        int udpPacketSize = receiveUdpPacket(udpPacketBuffer);
        if (udpPacketSize > 0)
        {
            TEENSY_SERIAL.write(udpPacketBuffer, udpPacketSize);
        }
        sendPeriodicDataToEvents();

        checkIfTeensyIsResponding();
    }
}
#endif