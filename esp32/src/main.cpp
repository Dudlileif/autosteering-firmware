#ifndef _MAIN_CPP
#define _MAIN_CPP

#include <Arduino.h>
#include <LittleFS.h>

#include <WiFiConfig.h>

#include "Config/Config.h"
#include "TeensyComms/TeensyComms.h"
#include "WebServer/WebServer.h"
#include "OTAUpdate/OTAUpdate.h"
#include "Network/Network.h"

void setup()
{
  pinMode(PRIORITY_MESSAGE_SIGNAL_PIN, OUTPUT);
  pinMode(WIFI_TOGGLE_PIN, INPUT);
  pinMode(WIFI_LED_R, OUTPUT);
  pinMode(WIFI_LED_G, OUTPUT);
  pinMode(WIFI_LED_B, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SEND_LED_PIN, OUTPUT);

  digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, HIGH);

  Serial.begin(SERIAL_BAUD);
  Serial.printf("ESP32 Firmware date: %s\n", firmwareDate);

  TEENSY_SERIAL.setRxBufferSize(256);
  TEENSY_SERIAL.begin(TEENSY_BAUD, SERIAL_8N1, RXD2, TXD2);

  mountFileSystem();
  motorConfig.loadFromFile(&LittleFS);
  wifiConfig.load(&LittleFS);

  Serial.printf("UDP ports:\n\tSend: %d\n\tRecieve: %d\n", wifiConfig.udpSendPort, wifiConfig.udpReceivePort);
  Serial.printf("TCP ports:\n\tSend: %d\n\tRecieve: %d\n", wifiConfig.tcpSendPort, wifiConfig.tcpReceivePort);

  Serial.println("Saved WiFi config json:");
  wifiConfig.printToStreamPretty(&Serial);

  Serial.println("Saved Motor config json:");
  motorConfig.printToStreamPretty(&Serial);

  delay(100);

  sendMotorConfig();

  getTeensyCrashReport(true);

  getTeensyFirmwareVersion(true);
  if (wifiConfig.startInAPMode || strlen(wifiConfig.ssid[0]) == 0)
  {
    startWiFiAP();
  }
  else
  {
    startWiFiClient();
  }
  receiveUDP.begin(wifiConfig.udpReceivePort);

  startWebServer();
}

void loop()
{
  if (doUpdate)
  {
    attemptToUpdate();
  }

  bool clientsAlive = checkHeartbeats();

  checkWiFiStatus();

  if (!uploadingFile && destinations && !priorityMessageInProgress)
  {

    uint8_t buffer[100];
    int serialSize = readSerial(buffer);
    if (serialSize > 0)
    {
      // for (int i = 0; i < serialSize; i++) {
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

    uint8_t tcpPacketBuffer[4096];
    int tcpPacketSize = receiveTcpPacket(tcpPacketBuffer);
    if (tcpPacketSize > 0)
    {
      TEENSY_SERIAL.write(tcpPacketBuffer, tcpPacketSize);
    }

    checkIfTeensyIsResponding();
  }
}

#endif