#ifndef TEENSYCOMMS_H
#define TEENSYCOMMS_H

#include <MotorConfig.h>

const int TEENSY_BAUD = 921600;

const int TEENSY_SERIAL_TIMEOUT_US = 1000000; // 1.0 s

extern String teensyCrashReport;

extern bool teensyUnresponsive;

extern MotorConfig motorConfig;

extern unsigned long teensyUptimeMs;

extern unsigned long lastTeensyCommTime;

void sendMotorConfig();

bool getTeensyFirmwareVersion(bool debugPrint);

bool getTeensyCrashReport(bool debugPrint);

bool getTeensyUptime(bool debugPrint);

void rebootTeensy();

int readSerial(uint8_t *buffer);

void checkIfTeensyIsResponding();

#endif