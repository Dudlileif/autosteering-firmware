#include "TeensyComms.h"

#include "Config/Config.h"
#include "OTAUpdate/OTAUpdate.h"

MotorConfig motorConfig;

String teensyCrashReport;

unsigned long teensyUptimeMs = 0;

unsigned long lastTeensyCommTime = 0;

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

    uint32_t startTime = millis();
    bool messageReady = false;
    while (millis() - startTime < 1000 && !messageReady)
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
        teensyFirmwareDate = TEENSY_SERIAL.readStringUntil('\n');
        if (debugPrint)
        {
            Serial.print("Teensy firwmare date:\n\t");
            Serial.println(teensyFirmwareDate);
        }
    }
    else if (debugPrint)
    {
        Serial.println("Failed to get Teensy firmware version.");
    }
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, LOW);
    priorityMessageInProgress = false;
    lastTeensyCommTime = micros();
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

    uint32_t startTime = millis();
    bool messageReady = false;
    while (millis() - startTime < 1000 && !messageReady)
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
    lastTeensyCommTime = micros();
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

    uint32_t startTime = millis();
    bool messageReady = false;
    while (millis() - startTime < 1000 && !messageReady)
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
    lastTeensyCommTime = micros();
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

int readSerial(uint8_t *buffer)
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
        lastTeensyCommTime = micros();
    }

    return size;
}

void checkIfTeensyIsResponding()
{
    if (micros() - lastTeensyCommTime > TEENSY_SERIAL_TIMEOUT_US && !priorityMessageInProgress)
    {
        teensyUnresponsive = true;
        Serial.printf("Teensy serial timeout reached (%d ms): %d ms\r", TEENSY_SERIAL_TIMEOUT_US / 1000, (micros() - lastTeensyCommTime) / 1000);
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
