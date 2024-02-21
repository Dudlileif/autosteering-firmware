#ifndef SENSORS_H
#define SENSORS_H

#include <ArduinoJson.h>

#include "Config/Config.h"
#include "Adafruit_BNO08x_RVC.h"

extern unsigned long sensorPrevUpdateTime;

extern Adafruit_BNO08x_RVC imuRVC;

extern BNO08x_RVC_Data currentImuReading;
extern BNO08x_RVC_Data prevImuReading;

extern uint16_t wasReading;

double roundToNumberOfDecimals(double, int);

void imuInit();

void wasInit();

void updateImuReading();

JsonDocument getSensorData();

#endif
