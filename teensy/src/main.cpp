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

#include <Arduino.h>

#include <Adafruit_BNO08x_RVC.h>
#include <ArduinoJson.h>
#include <pins_arduino.h>
#include <TMCStepper.h>
#include <ADC.h>

#include <MultiStream.h>
#include "Config/Config.h"

#include "Comms/Comms.h"
#include "OTAUpdate/OTAUpdate.h"
#include "StepperMotor/StepperMotor.h"
#include "Sensors/Sensors.h"

void setup()
{
  pinMode(PRIORITY_MESSAGE_SIGNAL_PIN, INPUT);
  pinMode(PIN_WAS, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  commsInit();
  imuInit();
  wasInit();
  stepperInit();

  if (CrashReport)
  {
    Serial.println("Crash report found, sending...");
    Serial.print(CrashReport);
  }
}

void loop()
{
  while (priorityModeEnabled())
  {
    handlePriorityMessage();
  }

  updateStepper();
  updateImuReading();
  sendSensorData();
}

// Handle incoming USB/Serial data
void serialEvent()
{
  if (!priorityModeEnabled())
  {
    receiveUSBSerialData();
  }
}

// Handle incoming network serial data.
void serialEvent2()
{
  if (!priorityModeEnabled())
  {
    receiveNetworkData();
  }
}

// Handle incoming GNSS serial data
void serialEvent3()
{
  if (!priorityModeEnabled())
  {
    receiveGNSSData();
  }
}
