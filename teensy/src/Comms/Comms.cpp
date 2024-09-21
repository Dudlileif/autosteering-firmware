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

#include "Comms.h"

#include "Config/Config.h"
#include "OTAUpdate/OTAUpdate.h"
#include "Sensors/Sensors.h"
#include "StepperMotor/StepperMotor.h"

DMAMEM byte networkSerialReadBuffer[1024];
DMAMEM byte networkSerialWriteBuffer[256];

DMAMEM byte gnssSerialWriteBuffer[1024];

MultiStream usbAndNetworkSerial = MultiStream(&Serial, &NETWORK_SERIAL);

boolean enableUSBSerial = false;

enum MessageType
{
    none,
    rtcm,
    json
};

char serialMessage[1024];
uint16_t serialMessageLength = 0;
MessageType serialMessageType = none;
uint16_t serialMessageRtcmLength = 0;
uint8_t serialMessageBracketOpenCount = 0;
uint8_t serialMessageBracketCloseCount = 0;

char networkMessage[1024];
uint16_t networkMessageLength = 0;
MessageType networkMessageType = none;
uint16_t networkMessageRtcmLength = 0;
uint8_t networkMessageBracketOpenCount = 0;
uint8_t networkMessageBracketCloseCount = 0;

char gnssMessage[128];
uint8_t gnssMessageLength = 0;
bool gnssSerialFinished = true;

void commsInit()
{
    Serial.begin(USB_BAUD);          // USB, communication to PC
    NETWORK_SERIAL.begin(DATA_BAUD); // communication with ESP32 serial
    GNSS_SERIAL.begin(GNSS_BAUD);    // communication to GNSS serial
    IMU_SERIAL.begin(IMU_BAUD);      // communication to IMU serial

    // Add larger buffer so we can receive chuncked RTCM messages
    NETWORK_SERIAL.addMemoryForRead(networkSerialReadBuffer, 1024);
    NETWORK_SERIAL.addMemoryForWrite(networkSerialWriteBuffer, 256);

    GNSS_SERIAL.addMemoryForWrite(gnssSerialWriteBuffer, 1024);

    Serial.printf("Teensy firmware version: %s | %s | %s\n", FIRMWARE_TYPE, VERSION, BUILD_TIMESTAMP);

    Serial.println("Serial connections initialized.");
}

void handlePriorityMessage()
{
    NETWORK_SERIAL.clear();
    if (!priorityMessageInProgress)
    {
        Serial.println("Priority message available, preparing...");
    }
    priorityMessageInProgress = true;
    elapsedMillis waiting;
    while (waiting < 1000)
    {
        const char byte = char(NETWORK_SERIAL.read());
        if (byte == '$')
        {
            const char *message = NETWORK_SERIAL.readStringUntil('\n').c_str();
            if (strlen(message) > 0)
            {
                Serial.println(message);
                Serial.printf("Priority message: \n\t%s\n", message);
                if (strstr(message, "FIRMWARE"))
                {
                    NETWORK_SERIAL.println("READY");
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
                    updateStepperDriverConfig();
                }
                else if (strstr(message, "VERSION"))
                {
                    NETWORK_SERIAL.println("TEENSY VERSION");
                    Serial.println("Sending firmware version");
                    NETWORK_SERIAL.println(String(FIRMWARE_TYPE) + String("_") + String(VERSION));
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
                else if (strstr(message, "UPTIME"))
                {
                    NETWORK_SERIAL.println("TEENSY UPTIME MS");
                    Serial.println("Sending uptime ms");
                    NETWORK_SERIAL.println(millis());
                }
                else if (strstr(message, "REBOOT"))
                {
                    Serial.println("Rebooting...");
                    SCB_AIRCR = 0x05FA0004;
                }
            }
        }
    }
    Serial.println("Priority message over.");
    priorityMessageInProgress = false;
}

void sendToProgram(char *message, int messageSize)
{
    if (enableUSBSerial)
    {
        Serial.write(message, messageSize);
        Serial.print("\r\n");
        Serial.send_now();
    }
    NETWORK_SERIAL.write(message, messageSize);
    NETWORK_SERIAL.print("\r\n");
}

void sendSensorData()
{
    if (sensorPrevUpdateElapsedTime > SENSOR_PERIOD_US)
    {
        JsonDocument data = getSensorData();
        int size = measureJson(data);
        if (size > 4)
        {
            char serialized[300];
            serializeJson(data, serialized);
            if (char(serialized[0]) == '{' && char(serialized[size - 1]) == '}')
            {
                sendToProgram(serialized, size);
                sensorPrevUpdateElapsedTime = 0;
            }
        }
    }
}

void receiveGNSSData()
{

    if (GNSS_SERIAL.available())
    {
        gnssMessage[gnssMessageLength] = GNSS_SERIAL.read();
        gnssMessageLength++;
        gnssSerialFinished = false;
    }
    if (gnssMessageLength > 1)
    {
        bool startsCorrectly = char(gnssMessage[0]) == '$';
        bool endsCorrectly = char(gnssMessage[gnssMessageLength - 2]) == '\r' && char(gnssMessage[gnssMessageLength - 1]) == '\n';

        // Serial.printf("Starts: %d, Ends: %d, Done:%d, Length:%d\n", startsCorrectly, endsCorrectly, gnssSerialFinished, gnssMessageLength);

        if (startsCorrectly && endsCorrectly)
        {
            sendToProgram(gnssMessage, gnssMessageLength);
            gnssMessageLength = 0;
            gnssSerialFinished = true;
        }

        else if (!startsCorrectly)
        {
            gnssMessageLength = 0;
            gnssSerialFinished = true;
        }
    }
}

void handleIncomingData(
    Stream *stream,
    char *message,
    uint16_t &messageLength,
    MessageType &messageType,
    uint16_t &messageRtcmLength,
    uint8_t &messageBracketOpenCount,
    uint8_t &messageBracketCloseCount)
{
    if (stream->available())
    {
        int byte = stream->read();
        // Look for message starting bytes.
        if (messageType == none)
        {
            // Look for first byte of RTCM.
            if (byte == 0xD3)
            {
                // Serial.println("RTCM message found");
                messageType = rtcm;
                message[0] = byte;
                messageLength = 1;
                GNSS_SERIAL.write(byte);
                return;
            }
            // Look for first byte of a JSON document.
            else if (char(byte) == '{')
            {
                // Serial.println("JSON message found");
                messageType = json;
                message[0] = byte;
                messageLength = 1;
                messageBracketOpenCount = 1;
                messageBracketCloseCount = 0;
                return;
            }
            // Go to next byte to look for a new message start.
            return;
        }
        // If we reach this point, a new message has been found,
        // so we can start handling the rest of the message bytes.

        // Add byte to current message.
        if (messageType != none)
        {
            message[messageLength] = byte;
            messageLength++;
        }

        if (messageType == rtcm)
        {
            GNSS_SERIAL.write(byte);
            // Get length from packet header
            if (messageLength == 3)
            {

                messageRtcmLength = (message[1] << 8 | message[2]);

                // Serial.println(message[1], BIN);
                // Serial.print(message[1] << 8, BIN);
                // Serial.print(" + ");
                // Serial.println(message[2], BIN);
                // Serial.print(message[1] << 8, 10);
                // Serial.print(" + ");
                // Serial.println(message[2], 10);
                Serial.printf("RTCM length: %4d\n", messageRtcmLength);
                if (messageRtcmLength > 1023)
                {
                    Serial.println("RTCM length too large, discarding.");
                    messageType = none;
                    messageRtcmLength = 0;
                    messageLength = 0;
                }
            }
            else if (messageLength == (3 + messageRtcmLength + 3))
            {
                Serial.println("RTCM message end found.");
                messageType = none;
                messageRtcmLength = 0;
                messageLength = 0;
            }
            else if (messageLength > (3 + messageRtcmLength + 3))
            {
                Serial.println("RTCM message end not found.");
                messageType = none;
                messageRtcmLength = 0;
                messageLength = 0;
            }
        }
        else if (messageType == json)
        {
            if (char(byte) == '{')
            {
                messageBracketOpenCount++;
            }
            else if (char(byte) == '}')
            {
                messageBracketCloseCount++;
            }
            if (messageBracketOpenCount == messageBracketCloseCount)
            {
                JsonDocument document;
                deserializeJson(document, message, messageLength);
                if (document.size() > 0)
                {
                    handleMotorControls(document);
                }
                // serializeJsonPretty(document, Serial);
                messageLength = 0;
                messageType = none;
            }
        }
        if (messageLength >= 1024)
        {
            Serial.println("Network message overloaded, clearing...");
            for (int i = 0; i < 1024; i++)
            {
                Serial.print(char(message[i]));
            }
            Serial.println();
            messageLength = 0;
            messageType = none;
        }
    }
}

void receiveNetworkData()
{
    handleIncomingData(
        &NETWORK_SERIAL,
        networkMessage,
        networkMessageLength,
        networkMessageType,
        networkMessageRtcmLength,
        networkMessageBracketOpenCount,
        networkMessageBracketCloseCount);
}

void receiveUSBSerialData()
{
    handleIncomingData(
        &Serial,
        serialMessage,
        serialMessageLength,
        serialMessageType,
        serialMessageRtcmLength,
        serialMessageBracketOpenCount,
        serialMessageBracketCloseCount);
}
