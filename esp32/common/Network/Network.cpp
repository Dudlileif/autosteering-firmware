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

#include "Network.h"
#include "../Config/Config.h"
#include <elapsedMillis.h>

WiFiConfig wifiConfig;

WiFiUDP sendUDP;
WiFiUDP receiveUDP;

WiFiMulti wifiMulti;

Destination destinations[4];
AsyncClient *tcpClient = nullptr;

wl_status_t prevWiFiClientStatus = WL_DISCONNECTED;

elapsedMillis sendLEDTime;

elapsedMillis hardwareIdentifierSendTime;

elapsedMillis lastAPConnectedDeviceDisconnectTime;

void setWiFiLED(color_t color)
{
#ifndef STEPPER_MOTOR_TESTING
    analogWrite(WIFI_LED_R, color.red);
    analogWrite(WIFI_LED_G, color.green);
    analogWrite(WIFI_LED_B, color.blue);
#endif
}

void setupMDNS()
{
    if (!MDNS.begin(wifiConfig.hostname))
    {
        Serial.println("Error setting up MDNS responder!");
        while (1)
        {
            delay(1000);
        }
    }
    Serial.printf("ESP hostname: %s\n", wifiConfig.hostname);
}

void startWiFiAP()
{
    setWiFiLED(colorYellow);

    WiFi.disconnect();
    WiFi.mode(WIFI_MODE_APSTA);
    Serial.println();
    Serial.println("Configuring access point...");

    // You can remove the password parameter if you want the AP to be open.
    // a valid password must have more than 7 characters
    if (!WiFi.softAP(String(wifiConfig.apSSID), String(wifiConfig.apPassword)))
    {
        log_e("Soft AP creation failed.");
        while (1)
            ;
    }
    Serial.printf("AP IP address: %s\n", WiFi.softAPIP().toString().c_str());
    setupMDNS();
}

void startWiFiClient()
{
    setWiFiLED(colorRed);

    if (strlen(wifiConfig.ssid[0]) == 0)
    {
        Serial.println("No WiFi client SSIDs set up, reverting to AP mode");
        return;
    }

    WiFi.softAPdisconnect();
    WiFi.mode(WIFI_MODE_STA);

    Serial.println("\nKnown networks:");
    for (int i = 0; i < 5; i++)
    {
        if (strlen(wifiConfig.ssid[i]) > 0)
        {
            wifiMulti.addAP(wifiConfig.ssid[i], strlen(wifiConfig.password[i]) > 0 ? wifiConfig.password[i] : NULL);
            Serial.printf("\t%s\n", wifiConfig.ssid[i]);
        }
    }
    int foundNetworks = WiFi.scanNetworks();
    Serial.printf("Networks in range: %d\n", foundNetworks);
    for (int i = 0; i < foundNetworks; i++)
    {
        Serial.printf("\tSSID: %-32s | AUTH: %-15s| RSSI: %d\n", WiFi.SSID(i).c_str(), wifiAuthTypeToString(WiFi.encryptionType(i)), WiFi.RSSI(i));
    }

    wifiMulti.run();
    setupMDNS();
}

static void handleData(void *, AsyncClient *, void *data, size_t len)
{
#ifdef BASE_STATION_RELAY
    GNSS_SERIAL.write((char *)data, len);
#endif
#ifdef AUTOSTEERING_BRIDGE
    TEENSY_SERIAL.write((char *)data, len);
#endif
}

static void handleError(void *, AsyncClient *client, uint8_t error)
{
    Serial.printf("\nConnection error %s from client %s\n", client->errorToString(error), client->remoteIP().toString().c_str());
}

static void handleDisconnect(void *arg, AsyncClient *client)
{
    Serial.printf("\nClient %s disconnected\n", client->remoteIP().toString().c_str());
    tcpClient = nullptr;
}

static void handleTimeOut(void *arg, AsyncClient *client, uint32_t time)
{
    Serial.printf("\nClient ACK timeout ip: %s\n", client->remoteIP().toString().c_str());
}

static void handleNewClient(void *arg, AsyncClient *client)
{
    Serial.printf("\nNew client has been connected to server, ip: %s\n", client->remoteIP().toString().c_str());
    client->onData(&handleData, NULL);
    client->onError(&handleError, NULL);
    client->onDisconnect(&handleDisconnect, NULL);
    client->onTimeout(&handleTimeOut, NULL);
    if (tcpClient != nullptr)
    {
        tcpClient->close(true);
    }
    tcpClient = client;
}

void setupTCP()
{
    AsyncServer *socketServer = new AsyncServer(wifiConfig.tcpSendPort);
    socketServer->onClient(&handleNewClient, socketServer);
    socketServer->begin();
}

void checkWiFiStatus()
{
    int clientsAlive = checkHeartbeats();
    checkSendLED();
    if (WiFi.getMode() == WIFI_MODE_STA)
    {
        wl_status_t status = (wl_status_t)wifiMulti.run(5000);
        if (prevWiFiClientStatus != status)
        {
            Serial.printf("WiFi client status: %s\n", wifiClientStatusToString(status).c_str());
        }
        if (status == WL_DISCONNECTED)
        {
            Serial.println("No network found, starting AP mode.");
            startWiFiAP();
        }
        else if (status == WL_CONNECTED)
        {
            setWiFiLED(colorGreen);
            if (status != prevWiFiClientStatus)
            {
                Serial.printf("WiFi connected to: %s\n", WiFi.SSID().c_str());
                Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
                Serial.printf("Gateway IP address: %s\n", WiFi.gatewayIP().toString().c_str());
                setupMDNS();
            }

#if defined(AUTOSTEERING_BRIDGE) || defined(AUTOSTEERING_REMOTE_CONTROL)
            if (hardwareIdentifierSendTime > 10000)
            {
                bool gatewayIsClient = false;
                IPAddress gatewayIPAddress = WiFi.gatewayIP();
                for (int i = 0; i < 4; i++)
                {
                    if (destinations[i].ip == gatewayIPAddress)
                    {
                        gatewayIsClient = true;
                        break;
                    }
                }
                if (!gatewayIsClient)
                {

                    Serial.printf("Sending identifier message to: %s:%d\n", WiFi.gatewayIP().toString().c_str(), wifiConfig.udpSendPort);
#ifdef AUTOSTEERING_BRIDGE
                    sendUdpPacket("Steering hardware", 18, WiFi.gatewayIP(), wifiConfig.udpSendPort);
#endif
#ifdef AUTOSTEERING_REMOTE_CONTROL
                    sendUdpPacket("Remote control", 15, WiFi.gatewayIP(), wifiConfig.udpSendPort);
#endif
                }
                hardwareIdentifierSendTime = 0;
            }
#endif
        }
        else
        {
            setWiFiLED(colorRed);
            if (status != prevWiFiClientStatus)
            {
                Serial.println("WiFi disconnected");
            }
        }
        prevWiFiClientStatus = status;
    }
    else if (WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA)
    {
        uint connected = WiFi.softAPgetStationNum();
        if (!wifiConfig.startInAPMode && wifiConfig.hasKnownNetworks() && connected < 1 && lastAPConnectedDeviceDisconnectTime > 5000)
        {
            if (wifiMulti.run(5000) == WL_CONNECTED)
            {
                WiFi.mode(WIFI_MODE_STA);
            }
        }
        static uint prevConnected = 0;
        if (connected < 1)
        {
            setWiFiLED(colorPink);
        }
        else
        {
            setWiFiLED(colorBlue);
        }
        if (prevConnected != connected)
        {
            Serial.printf("Connected devices: %d", connected);
            Serial.println();
            prevConnected = connected;

            if (connected < 1)
            {
                setWiFiLED(colorYellow);
                lastAPConnectedDeviceDisconnectTime = 0;
            }
            else
            {
                setWiFiLED(colorBlue);
            }
        }
    }
}

String wifiClientStatusToString(wl_status_t status)
{
    String name;
    switch (status)
    {
    case WL_IDLE_STATUS:
        name = String("Idle");
        break;
    case WL_NO_SSID_AVAIL:
        name = String("No SSID available");
        break;
    case WL_SCAN_COMPLETED:
        name = String("Scan completed");
        break;
    case WL_CONNECTED:
        name = String("Connected");
        break;
    case WL_CONNECT_FAILED:
        name = String("Connection failed");
        break;
    case WL_CONNECTION_LOST:
        name = String("Connection lost");
        break;
    case WL_DISCONNECTED:
        name = String("Disconnected");
        break;
    default:
        name = String("Unknown");
        break;
    }
    return name;
}

String wifiAuthTypeToString(wifi_auth_mode_t authType)
{
    String name;
    switch (authType)
    {
    case WIFI_AUTH_OPEN:
        name = String("Open");
        break;
    case WIFI_AUTH_WEP:
        name = String("WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        name = String("WPA-PSK");
        break;
    case WIFI_AUTH_WPA2_PSK:
        name = String("WPA2-PSK");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        name = String("WPA_WPA2-PSK");
        break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
        name = String("WPA2-ENTERPRISE");
        break;
    case WIFI_AUTH_WPA3_PSK:
        name = String("WPA3-PSK");
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        name = String("WPA2_WPA3-PSK");
        break;
    case WIFI_AUTH_WAPI_PSK:
        name = String("WAPI-PSK");
        break;
    default:
        name = String("Unknown");
        break;
    }
    return name;
}

int findDestinationIndexToOverwrite()
{
    int freeIndex = 0;
    for (int i = 0; i < 4; i++)
    {
        if (destinations[i].heartbeat == 0)
        {
            freeIndex = i;
            break;
        }
        else if (i > 0 && destinations[i].heartbeat < destinations[freeIndex].heartbeat)
        {
            freeIndex = i;
        }
    }
    return freeIndex;
}

void updateDestinations()
{
    Destination dest = Destination();
    dest.ip = receiveUDP.remoteIP();
    dest.heartbeat = millis();

    for (int i = 0; i < 4; i++)
    {
        if (destinations[i].ip == dest.ip)
        {
            // Already have this in our system, update
            // the heartbeat time
            destinations[i] = dest;
            return;
        }
    }

    int indexToReplace = findDestinationIndexToOverwrite();
    destinations[indexToReplace] = dest;
}

int checkHeartbeats()
{
    int clientsAlive = 0;
    for (int i = 0; i < 4; i++)
    {
        if (millis() - destinations[i].heartbeat < HEARTBEAT_BUFFER_MS)
        {
            clientsAlive++;
        }
        else
        {
            destinations[i].heartbeat = 0;
        }
    }
    return clientsAlive;
}

void sendUdpPacket(uint8_t *data, int packetSize, IPAddress destinationIP, uint destinationPort)
{

    if (sendUDP.beginPacket(destinationIP, destinationPort))
    {
#ifndef STEPPER_MOTOR_TESTING
        digitalWrite(SEND_LED_PIN, HIGH);
#endif
        sendLEDTime = 0;
        sendUDP.write(data, packetSize);
        sendUDP.endPacket();
    }
}

void sendUdpPacket(uint8_t *data, int packetSize, char *destinationHost, uint destinationPort)
{
    if (sendUDP.beginPacket(destinationHost, destinationPort))
    {
#ifndef STEPPER_MOTOR_TESTING
        digitalWrite(SEND_LED_PIN, HIGH);
#endif
        sendLEDTime = 0;
        sendUDP.write(data, packetSize);
        sendUDP.endPacket();
    }
}

void sendUdpPacket(const char *data, int packetSize, IPAddress destinationIP, uint destinationPort)
{

    if (sendUDP.beginPacket(destinationIP, destinationPort))
    {
#ifndef STEPPER_MOTOR_TESTING
        digitalWrite(SEND_LED_PIN, HIGH);
#endif
        sendLEDTime = 0;
        sendUDP.write((const uint8_t *)data, packetSize);
        sendUDP.endPacket();
    }
}

void sendUdpData(uint8_t *buffer, int messageSize)
{
    for (int i = 0; i < 4; i++)
    {
        if (millis() - destinations[i].heartbeat < HEARTBEAT_BUFFER_MS)
        {
            sendUdpPacket(buffer, messageSize, destinations[i].ip, wifiConfig.udpSendPort);
        }
    }
}

void sendUdpData(const char *buffer, int messageSize)
{
    for (int i = 0; i < 4; i++)
    {
        if (millis() - destinations[i].heartbeat < HEARTBEAT_BUFFER_MS)
        {
            sendUdpPacket(buffer, messageSize, destinations[i].ip, wifiConfig.udpSendPort);
        }
    }
}

int receiveUdpPacket(char *udpPacketBuffer)
{
    int size = receiveUDP.parsePacket();
    if (size > 0)
    {
        size = receiveUDP.read(udpPacketBuffer, size);
#if defined(AUTOSTEERING_BRIDGE) || defined(AUTOSTEERING_REMOTE_CONTROL) || defined(STEPPER_MOTOR_TESTING)
        updateDestinations();
#endif
    }
    return size;
}

IPAddress getIPAddress()
{
    IPAddress local = WiFi.localIP();

    if (local.toString() == "0.0.0.0")
    {
        return WiFi.softAPIP();
    }
    return local;
}

void checkSendLED()
{
    if (sendLEDTime > SEND_LED_ON_MS)
    {
#ifndef STEPPER_MOTOR_TESTING
        digitalWrite(SEND_LED_PIN, LOW);
#endif
    }
}