#ifndef WIFICONFIG_H
#define WIFICONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>

#define DEFAULT_HOSTNAME "autosteering"
#define DEFAULT_AP_SSID "Tractor Autosteering AP"
#define DEFAULT_AP_PASSWORD "tractor-autosteering"
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
    char password[5][32];
    uint16_t apServerPort;
    uint16_t tcpReceivePort;
    uint16_t tcpSendPort;
    uint16_t udpReceivePort;
    uint16_t udpSendPort;
    bool startInAPMode;

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
        startInAPMode = true;
    };
    ~WiFiConfig(){};

    JsonDocument json();
    bool load(FS *fs);
    bool save(FS *fs);
    void printToStream(Stream *stream);
    void printToStreamPretty(Stream *stream);
};

#endif
