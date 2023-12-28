#ifndef TEENSYCOMMS_H
#define TEENSYCOMMS_H

#include <MotorConfig.h>

const int TEENSY_BAUD = 921600;

extern MotorConfig motorConfig;

void sendMotorConfig();

bool getTeensyFirmwareVersion(bool);

int readSerial(uint8_t *buffer);

#endif