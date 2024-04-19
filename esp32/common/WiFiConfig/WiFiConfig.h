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

#ifndef WIFICONFIG_H
#define WIFICONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>

#ifdef BASE_STATION_RELAY
#define DEFAULT_HOSTNAME "gnss-rtk-base-station"
#define DEFAULT_AP_SSID "Base Station AP"
#define DEFAULT_AP_PASSWORD "base-station"
#define DEFAULT_RTK_BASE_STATION_ADDRESS "pirtkbase.local"
#endif

#ifndef BASE_STATION_RELAY
#define DEFAULT_HOSTNAME "autosteering"
#define DEFAULT_AP_SSID "Tractor Autosteering AP"
#define DEFAULT_AP_PASSWORD "tractor-autosteering"
#endif

#define WIFI_CONFIG_FILE "/wifi_config"

#define DEFAULT_AP_SERVER_PORT 80U
#define DEFAULT_TCP_RECEIVE_PORT 9999
#define DEFAULT_TCP_SEND_PORT 9999
#define DEFAULT_UDP_RECEIVE_PORT 6666U
#define DEFAULT_UDP_SEND_PORT 3333U

class WiFiConfig
{
private:
public:
    char hostname[32];
    char apSSID[32];
    char apPassword[32];
    char ssid[5][32];
    char password[5][63];
    uint16_t apServerPort;
    uint16_t tcpReceivePort;
    uint16_t tcpSendPort;
    uint16_t udpReceivePort;
    uint16_t udpSendPort;
    bool startInAPMode;
    char rtkBaseStationAddress[32];

    WiFiConfig()
    {
        strcpy(hostname, DEFAULT_HOSTNAME);
        strcpy(apSSID, DEFAULT_AP_SSID);
        strcpy(apPassword, DEFAULT_AP_PASSWORD);
        apServerPort = DEFAULT_AP_SERVER_PORT;
        tcpReceivePort = DEFAULT_TCP_RECEIVE_PORT;
        tcpSendPort = DEFAULT_TCP_SEND_PORT;
        udpReceivePort = DEFAULT_UDP_RECEIVE_PORT;
        udpSendPort = DEFAULT_UDP_SEND_PORT;
        startInAPMode = false;
#ifdef BASE_STATION_RELAY
        strcpy(rtkBaseStationAddress, DEFAULT_RTK_BASE_STATION_ADDRESS);
#endif
    };
    ~WiFiConfig(){};

    JsonDocument json();
    bool load(FS *fs);
    bool save(FS *fs);
    void printToStream(Stream *stream);
    void printToStreamPretty(Stream *stream);
};

#endif
