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
#ifdef AUTOSTEERING_BRIDGE
#include "TeensyComms.h"

#include <Config.h>
#include <OTAUpdate.h>
#include "../TeensyOTAUpdateAdditions/TeensyOTAUpdateAdditions.h"
#include <Network.h>

MotorConfig motorConfig;

String teensyCrashReport;

unsigned long teensyUptimeMs = 0;

elapsedMillis lastTeensyCommElapsedTime = 0;

bool teensyUnresponsive = false;

enum MessageType
{
    none,
    nmea,
    json
};
char teensyMessage[512];
MessageType messageType = none;
uint16_t messageLength = 0;
uint16_t nmeaDelimiter = 0;
uint8_t bracketOpenCount = 0;
uint8_t bracketCloseCount = 0;

TeensyHardwareState teensyHardwareState;

String gnssNmeaGns;
String gnssNmeaGst;
String gnssNmeaVtg;

void sendMotorConfig()
{
    priorityMessageInProgress = true;
    Serial.println("\nTransferring Motor config to Teensy...");
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, HIGH);
    delay(100);
    TEENSY_SERIAL.println("$MOTOR");
    motorConfig.printToStream(&TEENSY_SERIAL);
    Serial.println("Motor config sent.");
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, LOW);
    priorityMessageInProgress = false;
}

bool getTeensyFirmwareVersion(bool debugPrint)
{
    priorityMessageInProgress = true;

    if (debugPrint)
    {
        Serial.println("Attempting to get Teensy firmware version.");
    }
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, HIGH);
    delay(100);
    TEENSY_SERIAL.println("$VERSION");

    elapsedMillis elapsedTime;
    bool messageReady = false;
    while (elapsedTime < 1000 && !messageReady)
    {
        const char *message = TEENSY_SERIAL.readStringUntil('\n').c_str();
        if (strstr(message, "TEENSY VERSION"))
        {
            messageReady = true;
            break;
        }
    }
    if (messageReady)
    {
        teensyFirmwareVersion = TEENSY_SERIAL.readStringUntil('\n');
        if (debugPrint)
        {
            Serial.print("Teensy firmware version:\n\t");
            Serial.println(teensyFirmwareVersion);
        }
    }
    else if (debugPrint)
    {
        Serial.println("Failed to get Teensy firmware version.");
    }
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, LOW);
    priorityMessageInProgress = false;
    lastTeensyCommElapsedTime = 0;
    return messageReady;
}

bool getTeensyCrashReport(bool debugPrint)
{
    priorityMessageInProgress = true;

    if (debugPrint)
    {
        Serial.println("Attempting to get Teensy crash report.");
    }
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, HIGH);
    delay(100);
    TEENSY_SERIAL.println("$CRASH");

    elapsedMillis elapsedTime;
    bool messageReady = false;
    while (elapsedTime < 1000 && !messageReady)
    {
        const char *message = TEENSY_SERIAL.readStringUntil('\n').c_str();
        if (strstr(message, "CRASH REPORT"))
        {
            messageReady = true;
            break;
        }
    }
    if (messageReady)
    {
        teensyCrashReport = TEENSY_SERIAL.readStringUntil('~');
        if (debugPrint)
        {
            Serial.print("Teensy crash report:\n\t");
            Serial.println(teensyCrashReport);
        }
    }
    else if (debugPrint)
    {
        Serial.println("Failed to get Teensy crash report.");
    }
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, LOW);
    priorityMessageInProgress = false;
    lastTeensyCommElapsedTime = 0;
    return messageReady;
}

bool getTeensyUptime(bool debugPrint)
{
    priorityMessageInProgress = true;

    if (debugPrint)
    {
        Serial.println("Attempting to get Teensy uptime.");
    }
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, HIGH);
    delay(100);
    TEENSY_SERIAL.println("$UPTIME");

    elapsedMillis elapsedTime;
    bool messageReady = false;
    while (elapsedTime < 1000 && !messageReady)
    {
        const char *message = TEENSY_SERIAL.readStringUntil('\n').c_str();
        if (strstr(message, "TEENSY UPTIME MS"))
        {
            messageReady = true;
            break;
        }
    }
    if (messageReady)
    {
        teensyUptimeMs = TEENSY_SERIAL.readStringUntil('\n').toInt();

        if (debugPrint)
        {
            Serial.print("Teensy uptime ms:\n\t");
            Serial.println(teensyUptimeMs);
        }
    }
    else if (debugPrint)
    {
        Serial.println("Failed to get Teensy uptime.");
    }
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, LOW);
    priorityMessageInProgress = false;
    lastTeensyCommElapsedTime = 0;
    return messageReady;
}

void rebootTeensy()
{
    priorityMessageInProgress = true;
    Serial.println("\nRebooting Teensy...");
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, HIGH);
    delay(100);
    TEENSY_SERIAL.println("$REBOOT");
    motorConfig.printToStream(&TEENSY_SERIAL);
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, LOW);
    priorityMessageInProgress = false;
}

void readTeensySerial()
{

    while (TEENSY_SERIAL.available() && !doUpdate)
    {
        lastTeensyCommElapsedTime = 0;
        parseTeensyData(char(TEENSY_SERIAL.read()));
    }
}

void checkIfTeensyIsResponding()
{
    if (lastTeensyCommElapsedTime > TEENSY_SERIAL_TIMEOUT_MS && !priorityMessageInProgress)
    {
        teensyUnresponsive = true;
        Serial.printf("Teensy serial timeout reached (%d ms): %d ms\r", TEENSY_SERIAL_TIMEOUT_MS, lastTeensyCommElapsedTime);
        digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, LOW);
    }
    else
    {
        if (teensyUnresponsive && !priorityMessageInProgress)
        {
            Serial.printf("\n\r");
            Serial.println("Teensy serial now responding.");
            getTeensyCrashReport(true);
            sendMotorConfig();
        }
        teensyUnresponsive = false;
    }
}

void handleJsonData(JsonDocument &document)
{
    JsonObject data = document.as<JsonObject>();
    JsonVariant motorEnabled = data["motor_enabled"];
    if (!motorEnabled.isNull())
    {
        teensyHardwareState.motorEnabled = motorEnabled;
    }
    JsonVariant motorStalled = data["motor_stalled"];
    if (!motorStalled.isNull())
    {
        teensyHardwareState.motorStalled = motorStalled;
    }
    JsonVariant rpm = data["motor_rpm"];
    if (!rpm.isNull())
    {
        teensyHardwareState.rpm = rpm;
    }
    JsonVariant currentScale = data["motor_cs"];
    if (!currentScale.isNull())
    {
        teensyHardwareState.currentScale = currentScale;
    }
    JsonVariant stallguardResult = data["motor_sg"];
    if (!stallguardResult.isNull())
    {
        teensyHardwareState.stallguardResult = stallguardResult;
    }
    JsonVariant wasReading = data["was"];
    if (!wasReading.isNull())
    {
        teensyHardwareState.wasReading = wasReading;
    }
    JsonVariant wasTarget = data["was_target"];
    if (!wasTarget.isNull())
    {
        teensyHardwareState.wasTarget = wasTarget;
    }
    JsonVariant yaw = data["yaw"];
    if (!yaw.isNull())
    {
        teensyHardwareState.yaw = yaw;
    }
    JsonVariant pitch = data["pitch"];
    if (!pitch.isNull())
    {
        teensyHardwareState.pitch = pitch;
    }
    JsonVariant roll = data["roll"];
    if (!roll.isNull())
    {
        teensyHardwareState.roll = roll;
    }
    JsonVariant accX = data["acc_x"];
    if (!accX.isNull())
    {
        teensyHardwareState.accX = accX;
    }
    JsonVariant accY = data["acc_y"];
    if (!accY.isNull())
    {
        teensyHardwareState.accY = accY;
    }
    JsonVariant accZ = data["acc_z"];
    if (!accZ.isNull())
    {
        teensyHardwareState.accZ = accZ;
    }
}

void parseTeensyData(char byte)
{
    if (messageLength >= 512)
    {
        Serial.println("Teensy serial buffer maxed");
        messageLength = 0;
        messageType = none;
    }
    // Look for message starting bytes.
    if (messageType == none)
    {
        // Look for first byte of an NMEA message.
        if (byte == '$')
        {
            // Serial.println("NMEA message found");
            messageType = nmea;
            teensyMessage[0] = byte;
            messageLength = 1;
            nmeaDelimiter = 0;
            return;
        }
        // Look for first byte of a JSON document.
        else if (byte == '{')
        {
            // Serial.println("JSON message found");
            messageType = json;
            teensyMessage[0] = byte;
            messageLength = 1;
            bracketOpenCount = 1;
            bracketCloseCount = 0;
            return;
        }
        // Go to next byte to look for a new message start.
        // Serial.println("Teensy message start not found");
        return;
    }
    // If we reach this point, a new message has been found,
    // so we can start handling the rest of the message bytes.

    // Add byte to current message.
    if (messageType != none)
    {
        teensyMessage[messageLength] = byte;
        messageLength++;
    }

    if (messageType == nmea)
    {
        if (nmeaDelimiter > 0 && nmeaDelimiter + 2 == messageLength)
        {
            String nmea = String(teensyMessage, messageLength);
            // Serial.printf("NMEA: %s\n", nmea.c_str());
            if (nmea.substring(3, 6) == String("GNS"))
            {
                gnssNmeaGns = nmea;
                // Serial.printf("GNS: %s\n", gnssNmeaGns.c_str());
            }
            else if (nmea.substring(3, 6) == String("GST"))
            {
                gnssNmeaGst = nmea;
                // Serial.printf("GST: %s\n", gnssNmeaGst.c_str());
            }
            if (nmea.substring(3, 6) == String("VTG"))
            {
                gnssNmeaVtg = nmea;
                // Serial.printf("VTG: %s\n", gnssNmeaVtg.c_str());
            }
            sendUdpData(teensyMessage, messageLength);

            nmeaDelimiter = 0;
            messageLength = 0;
            messageType = none;
        }
        else if (byte == '*')
        {
            nmeaDelimiter = messageLength;
        }
    }
    else if (messageType == json)
    {
        if (byte == '{')
        {
            bracketOpenCount++;
        }
        else if (byte == '}')
        {
            bracketCloseCount++;
        }
        if (bracketOpenCount == bracketCloseCount)
        {
            sendUdpData(teensyMessage, messageLength);

            // Parsing the json is too expensive when steering is enabled
            //
            // JsonDocument document;
            // deserializeJson(document, teensyMessage, messageLength);
            // if (document.size() > 0)
            // {
            //     handleJsonData(document);
            // }
            // serializeJson(document, Serial);
            // Serial.println();

            messageLength = 0;
            messageType = none;
            bracketOpenCount = 0;
            bracketCloseCount = 0;
        }
    }
}
#endif
