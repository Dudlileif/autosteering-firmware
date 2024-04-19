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

#ifdef BASE_STATION_RELAY
#include "../GnssComms/GnssComms.h"
#endif

#ifndef BASE_STATION_RELAY
#include "../TeensyComms/TeensyComms.h"
#endif

#include "../WebServer/WebServer.h"
#include "../OTAUpdate/OTAUpdate.h"
#include "../Network/Network.h"

void mainSetup()
{
    pinMode(PRIORITY_MESSAGE_SIGNAL_PIN, OUTPUT);
    pinMode(WIFI_LED_R, OUTPUT);
    pinMode(WIFI_LED_G, OUTPUT);
    pinMode(WIFI_LED_B, OUTPUT);
    pinMode(SEND_LED_PIN, OUTPUT);
#ifdef BASE_STATION_RELAY
    pinMode(GNSS_READ_FAIL_LED_PIN, OUTPUT);
#endif

    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, HIGH);

    mountFileSystem();

    Serial.begin(SERIAL_BAUD);
    Serial.printf("ESP32 firmware version: %s | %s | %s\n", FIRMWARE_TYPE, VERSION, BUILD_TIMESTAMP);
    wifiConfig.load(&LittleFS);

    Serial.printf("UDP ports:\n\tSend: %d\n\tRecieve: %d\n", wifiConfig.udpSendPort, wifiConfig.udpReceivePort);
    Serial.printf("TCP ports:\n\tSend: %d\n\tRecieve: %d\n", wifiConfig.tcpSendPort, wifiConfig.tcpReceivePort);

    Serial.println("Saved WiFi config json:");
    wifiConfig.printToStreamPretty(&Serial);
#ifdef BASE_STATION_RELAY
    GNSS_SERIAL.setRxBufferSize(1024);
    GNSS_SERIAL.begin(115200, SERIAL_8N1, RXD2, TXD2);
#endif
#ifndef BASE_STATION_RELAY
    TEENSY_SERIAL.setRxBufferSize(512);
    TEENSY_SERIAL.begin(TEENSY_BAUD, SERIAL_8N1, RXD2, TXD2);
    motorConfig.loadFromFile(&LittleFS);

    Serial.println("Saved Motor config json:");
    motorConfig.printToStreamPretty(&Serial);

    delay(100);

    sendMotorConfig();

    getTeensyCrashReport(true);

    getTeensyFirmwareVersion(true);
#endif

    if (wifiConfig.startInAPMode || strlen(wifiConfig.ssid[0]) == 0)
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

void mainLoop()
{
    if (doUpdate)
    {
        attemptToUpdate();
    }

    bool clientsAlive = checkHeartbeats();

    checkWiFiStatus();

    if (!uploadingFile && !priorityMessageInProgress)
    {
#ifdef BASE_STATION_RELAY
        receiveGNSSData();
#endif
#ifndef BASE_STATION_RELAY
        uint8_t buffer[512];
        int serialSize = readTeensySerial(buffer);
        if (serialSize > 0)
        {
            // for (int i = 0; i < serialSize; i++)
            // {
            //   Serial.print((char)buffer[i]);
            // }
            // Serial.println();
            sendUdpData(buffer, serialSize);
        }

        // UDP maxes out at 1460
        char udpPacketBuffer[1460];

        int udpPacketSize = receiveUdpPacket(udpPacketBuffer);
        if (udpPacketSize > 0)
        {
            TEENSY_SERIAL.write(udpPacketBuffer, udpPacketSize);
        }

        checkIfTeensyIsResponding();
#endif
    }
}

#endif