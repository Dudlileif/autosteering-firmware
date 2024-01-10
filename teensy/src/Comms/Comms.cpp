#include "Comms.h"

#include "Config/Config.h"
#include "OTAUpdate/OTAUpdate.h"
#include "Sensors/Sensors.h"
#include "StepperMotor/StepperMotor.h"

DMAMEM byte networkSerialReadBuffer[4096];
DMAMEM byte networkSerialWriteBuffer[256];

MultiStream usbAndNetworkSerial = MultiStream(&Serial, &NETWORK_SERIAL);

boolean enableSerial = false;

char serialMessage[4096];
int serialMessageLength = 0;
bool serialMessageFinished = true;
bool serialMessageIsNotRTCM = true;

char networkMessage[4096];
int networkMessageLength = 0;
bool networkMessageFinished = true;
bool networkMessageIsNotRTCM = true;

char gnssMessage[128];
int gnssMessageLength = 0;
bool gnssSerialFinished = true;

void commsInit()
{
    Serial.begin(USB_BAUD);          // USB, communication to PC
    NETWORK_SERIAL.begin(DATA_BAUD); // communication with ESP32 serial
    GNSS_SERIAL.begin(GNSS_BAUD);    // communication to GNSS serial
    IMU_SERIAL.begin(IMU_BAUD);      // communication to IMU serial

    // Add larger buffer so we can receive chuncked RTCM messages
    NETWORK_SERIAL.addMemoryForRead(networkSerialReadBuffer, 4096);
    NETWORK_SERIAL.addMemoryForWrite(networkSerialWriteBuffer, 256);

    Serial.printf("Firmware date: %s\n", firmwareDate);

    Serial.println("Serial connections initialized.");
}

void sendToProgram(char *message, int messageSize)
{
    if (enableSerial)
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
    uint32_t now = micros();
    if (now - SENSOR_PERIOD_US > sensorPrevUpdateTime)
    {
        JsonVariantConst data = getSensorData();
        int size = measureJson(data);
        if (size > 4)
        {
            char serialized[256];
            serializeJson(data, serialized);
            if (char(serialized[0]) == '{' && char(serialized[size - 1]) == '}')
            {
                sendToProgram(serialized, size);
                sensorPrevUpdateTime = now;
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

void receiveNetworkData()
{
    if (NETWORK_SERIAL.available())
    {
        int byte = NETWORK_SERIAL.read();
        // Pass through all messages to GNSS to ensure all
        // RTCM messages gets through.
        GNSS_SERIAL.write(byte);
        networkMessageFinished = false;

        if (byte == 0xD3)
        {
            networkMessageIsNotRTCM = false;
        }
        else if (char(byte) == '{')
        {
            networkMessageIsNotRTCM = true;
        }

        if (networkMessageIsNotRTCM)
        {
            // Serial.println(char(byte));
            networkMessage[networkMessageLength] = byte;
            networkMessageLength++;

            if (char(byte) == '}')
            {
                networkMessageFinished = true;
                networkMessageIsNotRTCM = false;

                DynamicJsonDocument document(networkMessageLength);
                DeserializationError error = deserializeJson(document, networkMessage, networkMessageLength);
                if (document.size() > 0)
                {
                    handleMotorControls(document);
                }
                networkMessageLength = 0;
            }
        }
    }
}

void receiveUSBSerialData()
{
    if (Serial.available())
    {
        int byte = Serial.read();
        // Pass through all messages to GNSS to ensure all
        // RTCM messages gets through.
        GNSS_SERIAL.write(byte);

        serialMessageFinished = false;

        if (byte == 0xD3)
        {
            serialMessageIsNotRTCM = false;
        }
        else if (char(byte) == '{')
        {
            serialMessageIsNotRTCM = true;
        }

        if (serialMessageIsNotRTCM)
        {
            // Serial.println(char(byte));
            serialMessage[serialMessageLength] = byte;
            serialMessageLength++;

            if (char(byte) == '}')
            {
                serialMessageFinished = true;
                serialMessageIsNotRTCM = false;
                // Serial.write(serialMessage, serialMessageLength);

                DynamicJsonDocument document(serialMessageLength);
                DeserializationError error = deserializeJson(document, serialMessage, serialMessageLength);

                handleMotorControls(document);

                serialMessageLength = 0;
            }
        }
    }
}