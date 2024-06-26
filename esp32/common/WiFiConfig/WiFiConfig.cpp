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

#include "WiFiConfig.h"

#include <ArduinoJson.h>

bool WiFiConfig::load(FS *fs)
{
    bool configExists = fs->exists(WIFI_CONFIG_FILE);
    if (configExists)
    {
        Serial.println("WiFi config found.");
        JsonDocument savedWifiConfig;

        File file = fs->open(WIFI_CONFIG_FILE);

        deserializeJson(savedWifiConfig, file);

        Serial.println("Loading...");

        file.close();

        if (savedWifiConfig.isNull())
        {
            Serial.println("WiFi config is empty, starting in AP mode.");
            return false;
        }

        if (savedWifiConfig.containsKey("ap_ssid"))
        {
            strcpy(apSSID, savedWifiConfig["ap_ssid"]);
            Serial.printf("AP SSID: %s\n", apSSID);
        }

        if (savedWifiConfig.containsKey("ap_password"))
        {
            strcpy(apPassword, savedWifiConfig["ap_password"]);
            Serial.printf("AP Password: %s\n", apPassword);
        }

        if (savedWifiConfig.containsKey("ssid"))
        {
            JsonArray ssids = savedWifiConfig["ssid"];

            for (int i = 0; i < ssids.size(); i++)
            {
                if (ssids[i] == NULL)
                {
                    break;
                }
                strcpy(ssid[i], ssids[i]);
            }
            Serial.println("Saved SSIDs:");
            for (int i = 0; i < 5; i++)
            {
                if (strlen(ssid[i]) > 0)
                {
                    Serial.printf("\t%s\n", ssid[i]);
                }
            }
        }

        if (savedWifiConfig.containsKey("password"))
        {
            JsonArray passwords = savedWifiConfig["password"];

            for (int i = 0; i < passwords.size(); i++)
            {

                strcpy(password[i], passwords[i]);
            }
            Serial.println("Saved passwords:");
            for (int i = 0; i < 5; i++)
            {
                if (strlen(password[i]) > 0)
                {
                    Serial.printf("\t%s\n", password[i]);
                }
            }
        }
        if (savedWifiConfig.containsKey("hostname"))
        {
            strcpy(hostname, savedWifiConfig["hostname"]);
            Serial.printf("Saved hostname: %s\n", hostname);
        }

        if (savedWifiConfig.containsKey("ap_ssid"))
        {
            strcpy(apSSID, savedWifiConfig["ap_ssid"]);
            Serial.printf("Saved AP SSID: %s\n", apSSID);
        }
        if (savedWifiConfig.containsKey("ap_password"))
        {
            strcpy(apPassword, savedWifiConfig["ap_password"]);
            Serial.printf("Saved AP password: %s\n", apPassword);
        }

        if (savedWifiConfig.containsKey("ap_server_port"))
        {
            apServerPort = savedWifiConfig["ap_server_port"];
            Serial.printf("Saved AP server port: %d\n", apServerPort);
        }

        if (savedWifiConfig.containsKey("tcp_receive_port"))
        {
            tcpReceivePort = savedWifiConfig["tcp_receive_port"];
            Serial.printf("Saved TCP receive port: %d\n", tcpReceivePort);
        }

        if (savedWifiConfig.containsKey("tcp_send_port"))
        {
            tcpSendPort = savedWifiConfig["tcp_send_port"];
            Serial.printf("Saved TCP send port: %d\n", tcpSendPort);
        }

        if (savedWifiConfig.containsKey("udp_receive_port"))
        {
            udpReceivePort = savedWifiConfig["udp_receive_port"];
            Serial.printf("Saved UDP receive port: %d\n", udpReceivePort);
        }

        if (savedWifiConfig.containsKey("udp_send_port"))
        {
            udpSendPort = savedWifiConfig["udp_send_port"];
            Serial.printf("Saved UDP send port: %d\n", udpSendPort);
        }

        if (savedWifiConfig.containsKey("start_in_ap_mode"))
        {
            startInAPMode = savedWifiConfig["start_in_ap_mode"];
            Serial.printf("Start in AP mode: %s\n", startInAPMode ? "true" : "false");
        }

#ifdef BASE_STATION_RELAY
        if (savedWifiConfig.containsKey("rtk_base_station_address"))
        {
            strcpy(rtkBaseStationAddress, savedWifiConfig["rtk_base_station_address"]);
            Serial.printf("RTKBase station address:  %s\n", rtkBaseStationAddress);
        }
#endif

        Serial.println("WiFi config loaded successfully");

        return true;
    }
    Serial.println("WiFi config not found, starting in AP mode.");
    return false;
}

JsonDocument WiFiConfig::json()
{
    JsonDocument jsonDocument;

    JsonArray ssids = jsonDocument["ssid"].to<JsonArray>();
    JsonArray passwords = jsonDocument["password"].to<JsonArray>();

    for (int i = 0; i < 5; i++)
    {
        if (strlen(ssid[i]) == 0)
        {
            break;
        }
        ssids.add(ssid[i]);
        passwords.add(password[i]);
    }
    jsonDocument["hostname"] = hostname;
    jsonDocument["ap_ssid"] = apSSID;
    jsonDocument["ap_password"] = apPassword;
    jsonDocument["ap_server_port"] = apServerPort;
    jsonDocument["tcp_receive_port"] = tcpReceivePort;
    jsonDocument["tcp_send_port"] = tcpSendPort;
    jsonDocument["udp_receive_port"] = udpReceivePort;
    jsonDocument["udp_send_port"] = udpSendPort;
    jsonDocument["start_in_ap_mode"] = startInAPMode;
#ifdef BASE_STATION_RELAY
    jsonDocument["rtk_base_station_address"] = rtkBaseStationAddress;
#endif
    return jsonDocument;
}

bool WiFiConfig::save(FS *fs)
{
    JsonDocument jsonDocument;

    File file = fs->open(WIFI_CONFIG_FILE, FILE_WRITE);
    if (!file)
    {
        Serial.printf("Couldn't write file: %s\n", WIFI_CONFIG_FILE);
        return false;
    }
    serializeJson(json(), file);
    file.close();
    return true;
}

void WiFiConfig::printToStream(Stream *stream)
{

    serializeJson(json(), *stream);
}

void WiFiConfig::printToStreamPretty(Stream *stream)
{

    serializeJsonPretty(json(), *stream);
}

bool WiFiConfig::hasKnownNetworks()
{
    bool networksKnown = false;
    for (int i = 0; i < 5; i++)
    {
        if (strlen(ssid[i]) > 0)
        {
            networksKnown = true;
            break;
        }
    }
    return networksKnown;
}
