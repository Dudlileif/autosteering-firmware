#ifndef COMMS_H
#define COMMS_H

#include <Arduino.h>
#include <MultiStream.h>

extern MultiStream usbAndNetworkSerial;

extern boolean enableSerial;

extern DMAMEM byte networkSerialReadBuffer[4096];
extern DMAMEM byte networkSerialWriteBuffer[256];

extern char serialMessage[4096];
extern uint16_t serialMessageLength;
extern bool serialMessageFinished;
extern bool serialMessageIsRTCM;

extern char networkMessage[4096];
extern uint16_t networkMessageLength;
extern bool networkMessageFinished;
extern bool networkMessageIsRTCM;

extern char gnssMessage[128];
extern uint8_t gnssMessageLength;
extern bool gnssSerialFinished;

void commsInit();

void handlePriorityMessage();

void sendToProgram(char *, int);

void sendSensorData();

void receiveGNSSData();

void receiveNetworkData();

void receiveUSBSerialData();

#endif