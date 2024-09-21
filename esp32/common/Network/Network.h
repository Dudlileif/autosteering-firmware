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

#ifndef NETWORK_H
#define NETWORK_H

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiAP.h>
#include <WiFiUdp.h>
#include <AsyncTCP.h>
#include <WiFiConfig.h>
#include <ESPmDNS.h>
#include <../Config/Config.h>

extern WiFiConfig wifiConfig;

extern WiFiUDP sendUDP;
extern WiFiUDP receiveUDP;

extern WiFiMulti wifiMulti;

struct Destination
{
    IPAddress ip;
    long heartbeat;
};

extern Destination destinations[4];
extern AsyncClient *tcpClient;

extern wl_status_t prevWiFiClientStatus;

void setWiFiLED(color_t color);
void startWiFiAP();
void startWiFiClient();
void setupTCP();
void checkWiFiStatus();
String wifiClientStatusToString(wl_status_t status);
String wifiAuthTypeToString(wifi_auth_mode_t authType);
int findDestinationIndexToOverwrite();
void updateDestinations();
int checkHeartbeats();

IPAddress getIPAddress();

void sendUdpPacket(uint8_t *, int, IPAddress, uint);
void sendUdpPacket(uint8_t *, int, char *, uint);
void sendUdpPacket(const char *, int, IPAddress, uint);
void sendUdpData(uint8_t *, int);
void sendUdpData(const char *, int);
int receiveUdpPacket(char *);

void checkSendLED();
#endif