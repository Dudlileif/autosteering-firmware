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

MotorConfig motorConfig;

String teensyCrashReport;

unsigned long teensyUptimeMs = 0;

elapsedMillis lastTeensyCommElapsedTime = 0;

bool teensyUnresponsive = false;

void sendMotorConfig()
{
    priorityMessageInProgress = true;
    Serial.println("\nTransferring Motor config to Teensy...");
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, HIGH);
    delay(100);
    TEENSY_SERIAL.println("MOTOR");
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
    TEENSY_SERIAL.println("VERSION");

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
            Serial.print("Teensy firwmare date:\n\t");
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
    TEENSY_SERIAL.println("CRASH");

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
    TEENSY_SERIAL.println("UPTIME");

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
    TEENSY_SERIAL.println("REBOOT");
    motorConfig.printToStream(&TEENSY_SERIAL);
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, LOW);
    priorityMessageInProgress = false;
}

int readTeensySerial(uint8_t *buffer)
{
    int size = 0;
    while (TEENSY_SERIAL.available())
    {
        // if (size == 0) {
        //   Serial.println("-------------------");
        // }
        buffer[size] = TEENSY_SERIAL.read();
        // Serial.print(char(buffer[size]));
        size++;
        lastTeensyCommTime = millis();
    }

    return size;
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
#endif
