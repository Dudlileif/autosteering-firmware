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

        if (savedWifiConfig["ap_ssid"].is<const char *>())
        {
            strlcpy(apSSID, savedWifiConfig["ap_ssid"], 32);
            Serial.printf("AP SSID: %s\n", apSSID);
        }

        if (savedWifiConfig["ap_password"].is<const char *>())
        {
            strlcpy(apPassword, savedWifiConfig["ap_password"], 32);
            Serial.printf("AP Password: %s\n", apPassword);
        }

        if (savedWifiConfig["ssid"].is<JsonArray>())
        {
            JsonArray ssids = savedWifiConfig["ssid"];

            for (int i = 0; i < ssids.size(); i++)
            {
                if (ssids[i] == NULL)
                {
                    break;
                }
                strlcpy(ssid[i], ssids[i], 32);
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

        if (savedWifiConfig["password"].is<JsonArray>())
        {
            JsonArray passwords = savedWifiConfig["password"];

            for (int i = 0; i < passwords.size(); i++)
            {

                strlcpy(password[i], passwords[i], 63);
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
        if (savedWifiConfig["hostname"].is<const char *>())
        {
            strlcpy(hostname, savedWifiConfig["hostname"], 32);
            Serial.printf("Saved hostname: %s\n", hostname);
        }

        if (savedWifiConfig["ap_ssid"].is<const char *>())
        {
            strlcpy(apSSID, savedWifiConfig["ap_ssid"], 32);
            Serial.printf("Saved AP SSID: %s\n", apSSID);
        }
        if (savedWifiConfig["ap_password"].is<const char *>())
        {
            strlcpy(apPassword, savedWifiConfig["ap_password"], 32);
            Serial.printf("Saved AP password: %s\n", apPassword);
        }

        if (savedWifiConfig["ap_server_port"].is<uint16_t>())
        {
            apServerPort = savedWifiConfig["ap_server_port"];
            Serial.printf("Saved AP server port: %d\n", apServerPort);
        }

        if (savedWifiConfig["tcp_receive_port"].is<uint16_t>())
        {
            tcpReceivePort = savedWifiConfig["tcp_receive_port"];
            Serial.printf("Saved TCP receive port: %d\n", tcpReceivePort);
        }

        if (savedWifiConfig["tcp_send_port"].is<uint16_t>())
        {
            tcpSendPort = savedWifiConfig["tcp_send_port"];
            Serial.printf("Saved TCP send port: %d\n", tcpSendPort);
        }

        if (savedWifiConfig["udp_receive_port"].is<uint16_t>())
        {
            udpReceivePort = savedWifiConfig["udp_receive_port"];
            Serial.printf("Saved UDP receive port: %d\n", udpReceivePort);
        }

        if (savedWifiConfig["udp_send_port"].is<uint16_t>())
        {
            udpSendPort = savedWifiConfig["udp_send_port"];
            Serial.printf("Saved UDP send port: %d\n", udpSendPort);
        }

        if (savedWifiConfig["start_in_ap_mode"].is<bool>())
        {
            startInAPMode = savedWifiConfig["start_in_ap_mode"];
            Serial.printf("Start in AP mode: %s\n", startInAPMode ? "true" : "false");
        }

#ifdef BASE_STATION_RELAY
        if (savedWifiConfig["rtk_base_station_address"].is<const char *>())
        {
            strlcpy(rtkBaseStationAddress, savedWifiConfig["rtk_base_station_address"], 32);
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
