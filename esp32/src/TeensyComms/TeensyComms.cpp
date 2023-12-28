#include "TeensyComms.h"

#include "Config/Config.h"
#include "OTAUpdate/OTAUpdate.h"

MotorConfig motorConfig;

void sendMotorConfig()
{
    Serial.println("\nTransferring Motor config to Teensy...");
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, HIGH);
    delay(100);
    TEENSY_SERIAL.println("MOTOR");
    motorConfig.printToStream(&TEENSY_SERIAL);
    Serial.println("Motor config sent.");
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, LOW);
}

bool getTeensyFirmwareVersion(bool debugPrint)
{
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
    return messageReady;
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
    }
    // if (size > 0) {
    //   Serial.println("-------------------");
    // }

    // int size = TEENSY_SERIAL.available();
    // if (size > 0) {
    //   size = TEENSY_SERIAL.readBytesUntil('\r\n', buffer, size);

    //   for (int i = 0; i < size; i++) {
    //     Serial.print(char(buffer[i]));
    //   }
    //   Serial.println();
    // }
    return size;
}
