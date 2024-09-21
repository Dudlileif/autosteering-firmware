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

#ifndef _MAIN_CPP
#define _MAIN_CPP

#include <main.h>
#include <Network.h>
#include <LittleFS.h>

#include "Config/Config.h"
#include "StepperMotor/StepperMotor.h"
#include "WebServerMotorAdditions/WebServerMotorAdditions.h"
#include "Sensors/Sensors.h"

void sendSensorDataUDP();

void setup()
{
  mainSetup();
  initSensors();
  stepperInit();
  addMotorCallbacksToWebServer();

  motorConfig.loadFromFile(&LittleFS);
  Serial.println("Saved Motor config json:");
  motorConfig.printToStreamPretty(&Serial);

  delay(100);
}

void loop()
{
  if (mainLoop())
  {
    updateStepper();
    sendPeriodicDataToEvents();
    sendSensorDataUDP();
    uint8_t buffer[512];

    // UDP maxes out at 1460
    char udpPacketBuffer[1460];

    int udpPacketSize = receiveUdpPacket(udpPacketBuffer);
    if (udpPacketSize > 0)
    {
      JsonDocument document;
      deserializeJson(document, udpPacketBuffer, udpPacketSize);
      if (document.size() > 0)
      {
        handleMotorControls(document);
      }
    }
  }
}

const int SENSOR_PERIOD_US = 10000;

elapsedMicros sensorPrevUpdateElapsedTime;

void sendSensorDataUDP()
{
  if (sensorPrevUpdateElapsedTime > SENSOR_PERIOD_US)
  {
    JsonDocument data = getSensorData();
    int size = measureJson(data);
    if (size > 4)
    {
      char serialized[256];
      serializeJson(data, serialized);
      if (char(serialized[0]) == '{' && char(serialized[size - 1]) == '}')
      {
        sendUdpData(serialized, size);
        sensorPrevUpdateElapsedTime = 0;
      }
    }
  }
}
#endif
