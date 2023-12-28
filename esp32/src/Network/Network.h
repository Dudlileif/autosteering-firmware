#ifndef NETWORK_H
#define NETWORK_H

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiAP.h>
#include "WiFiUdp.h"
#include <AsyncTCP.h>
#include <AsyncUDP.h>
#include <WiFiConfig.h>
#include <ESPmDNS.h>

extern WiFiConfig wifiConfig;

extern WiFiUDP sendUDP;
extern WiFiUDP receiveUDP;

extern IPAddress myIP;

extern WiFiMulti wifiMulti;

struct Destination
{
    IPAddress ip;
    long heartbeat;
};

extern Destination destinations[4];
extern WiFiClient tcpClient;

extern wl_status_t prevWiFiClientStatus;

void setWiFiLED(int r, int g, int b);
void startWiFiAP();
void startWiFiClient();
void checkWiFiStatus();
int findDestinationIndexToOverwrite();
void updateDestinations();
bool checkHeartbeats();

IPAddress getIPAddress();

void sendUdpPacket(uint8_t *, int, IPAddress, uint);
void sendUdpData(uint8_t *, int);
int receiveUdpPacket(char *);
int receiveTcpPacket(uint8_t *);

#endif