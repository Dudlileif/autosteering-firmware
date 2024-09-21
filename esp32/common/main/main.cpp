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

#include <Arduino.h>
#include <LittleFS.h>

#include <WiFiConfig.h>

#include "../Config/Config.h"

#include "../WebServer/WebServer.h"
#include "../OTAUpdate/OTAUpdate.h"
#include "../Network/Network.h"

void mainSetup()
{
#ifndef STEPPER_MOTOR_TESTING
    pinMode(WIFI_LED_R, OUTPUT);
    pinMode(WIFI_LED_G, OUTPUT);
    pinMode(WIFI_LED_B, OUTPUT);
    pinMode(SEND_LED_PIN, OUTPUT);
#endif
    mountFileSystem();

    Serial.begin(SERIAL_BAUD);
    Serial.printf("ESP32 firmware version: %s | %s | %s\n", FIRMWARE_TYPE, VERSION, BUILD_TIMESTAMP);
    wifiConfig.load(&LittleFS);

    Serial.printf("UDP ports:\n\tSend: %d\n\tRecieve: %d\n", wifiConfig.udpSendPort, wifiConfig.udpReceivePort);
    Serial.printf("TCP ports:\n\tSend: %d\n\tRecieve: %d\n", wifiConfig.tcpSendPort, wifiConfig.tcpReceivePort);

    Serial.println("Saved WiFi config json:");
    wifiConfig.printToStreamPretty(&Serial);

    if (wifiConfig.startInAPMode || !wifiConfig.hasKnownNetworks())
    {
        startWiFiAP();
    }
    else
    {
        startWiFiClient();
    }
    receiveUDP.begin(wifiConfig.udpReceivePort);
    setupTCP();
    startWebServer();
}

bool mainLoop()
{
    if (doUpdate)
    {
        attemptToUpdate();
    }

    checkWiFiStatus();

    if (!uploadingFile && !priorityMessageInProgress)
    {
        return true;
    }
    return false;
}

#endif