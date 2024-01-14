#include <Arduino.h>

#include <Adafruit_BNO08x_RVC.h>
#include <Adafruit_ADS1X15.h>
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
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  commsInit();
  imuInit();
  wasInit();
  stepperInit();
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
