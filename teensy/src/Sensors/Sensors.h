#ifndef SENSORS_H
#define SENSORS_H

#include <ArduinoJson.h>

#include "Config/Config.h"
#include "Adafruit_BNO08x_RVC.h"
#include "Adafruit_ADS1X15.h"

extern unsigned long sensorPrevUpdateTime;

extern Adafruit_BNO08x_RVC imuRVC;

extern Adafruit_ADS1115 ads1115;

extern BNO08x_RVC_Data currentImuReading;
extern BNO08x_RVC_Data prevImuReading;

extern int16_t wasReading;

double roundToNumberOfDecimals(double, int);

void imuInit();

void wasInit();

void updateImuReading();

StaticJsonDocument<SENSOR_DATA_SIZE> getSensorData();

#endif
