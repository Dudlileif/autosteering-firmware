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

void handlePriorityMessage();

void setup()
{
  pinMode(PRIORITY_MESSAGE_SIGNAL_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

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

void handlePriorityMessage()
{
  NETWORK_SERIAL.clear();
  if (!priorityMessageInProgress)
  {
    Serial.println("Priority message available, preparing...");
  }
  priorityMessageInProgress = true;
  const char *message = NETWORK_SERIAL.readStringUntil('\n').c_str();
  if (strlen(message) > 0)
  {
    Serial.println(message);
    Serial.printf("Priority message: \n\t%s\n", message);
    if (strstr(message, "FIRMWARE"))
    {
      Serial.println("Firmware update available, preparing...");
      firmwareUpdateInProgress = true;
      performUpdate();
    }
    else if (strstr(message, "MOTOR"))
    {
      Serial.println("Attempting to receive motor config...");
      bool success = motorConfig.load(&NETWORK_SERIAL);
      Serial.printf("Motor config reception %s.\n", success ? "completed" : "failed");
      Serial.println("Motor config:");
      motorConfig.printToStreamPretty(&Serial);
      Serial.println();
      stepperInit();
    }
    else if (strstr(message, "VERSION"))
    {
      NETWORK_SERIAL.println("TEENSY VERSION");
      Serial.println("Sending firmware version");
      NETWORK_SERIAL.println(firmwareDate);
    }
    else if (strstr(message, "CRASH"))
    {
      NETWORK_SERIAL.print("CRASH REPORT");
      NETWORK_SERIAL.println("");
      NETWORK_SERIAL.print("CRASH REPORT");

      if (CrashReport)
      {
        Serial.println("Crash report found, sending...");
        NETWORK_SERIAL.print(CrashReport);
        Serial.print(CrashReport);
      }
      else
      {
        Serial.println("No crash report found.");
        NETWORK_SERIAL.print("No crash report.");
      }
      NETWORK_SERIAL.print('~');
      NETWORK_SERIAL.println("");
    }
    else if (strstr(message, "REBOOT"))
    {
      Serial.println("Rebooting...");
      SCB_AIRCR = 0x05FA0004;
    }
  }
  Serial.println("Priority message over.");
  priorityMessageInProgress = false;
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