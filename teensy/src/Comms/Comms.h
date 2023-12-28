#ifndef COMMS_H
#define COMMS_H

#include <Arduino.h>

extern boolean enableSerial;

extern DMAMEM byte networkSerialReadBuffer[4096];
extern DMAMEM byte networkSerialWriteBuffer[256];

extern char serialMessage[4096];
extern int serialMessageLength;
extern bool serialMessageFinished;
extern bool serialMessageIsNotRTCM;

extern char networkMessage[4096];
extern int networkMessageLength;
extern bool networkMessageFinished;
extern bool networkMessageIsNotRTCM;

extern char gnssMessage[128];
extern int gnssMessageLength;
extern bool gnssSerialFinished;

void commsInit();

void sendToProgram(char *, int);

void sendSensorData();

void receiveGNSSData();

void receiveNetworkData();

void receiveUSBSerialData();

#endif