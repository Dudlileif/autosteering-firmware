#include "OTAUpdate.h"

#include <Update.h>
#include <LittleFS.h>

#include "Config/Config.h"
#include "TeensyComms/TeensyComms.h"

String firmwareDate = "2024-02-19";
String teensyFirmwareDate;

bool priorityMessageInProgress = false;

bool doUpdate = false;

String updateFileName;

void mountFileSystem()
{
    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED))
    {
        Serial.println("LittleFS Mount Failed");
        return;
    }
}

void attemptToUpdate()
{ // Update ESP32
    if (updateFileName.endsWith(".bin"))
    {
        bool updated = attemptToUpdateSelf(updateFileName);
        if (updated)
        {
            Serial.println("ESP was updated.");
            ESP.restart();
        }
        else
        {
            Serial.println("ESP was NOT updated");
        }
    }
    // Update Teensy
    else if (updateFileName.endsWith(".hex"))
    {
        bool updated = attemptToUpdateTeensy(updateFileName);
        if (updated)
        {
            uint32_t waitStart = millis();
            Serial.println("Teensy was updated");
            bool gotVersion = false;
            Serial.print("Attempting to get version");
            do
            {
                delay(500);
                Serial.print(".");
                gotVersion = getTeensyFirmwareVersion(false);
            } while (!gotVersion && millis() - waitStart < 10000);
            Serial.println();
            if (gotVersion)
            {
                Serial.print("Teensy updated to version:\n\t");
                Serial.println(teensyFirmwareDate);
                sendMotorConfig();
            }
            else
            {
                Serial.println("Failed to find Teensy version.");
            }
        }
        else
        {
            Serial.println("Teensy was NOT updated");
        }
    }
    priorityMessageInProgress = false;
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, LOW);
    doUpdate = false;
}

bool performUpdate(Stream &updateSource, size_t updateSize)
{
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, HIGH);
    if (Update.begin(updateSize))
    {
        Update.onProgress([](size_t prg, size_t sz)
                          { Serial.printf("Progress: %d%%\r", int(100 * prg / sz)); });
        size_t written = Update.writeStream(updateSource);
        if (written == updateSize)
        {
            Serial.printf("Written: %d bytes successfully\n", written);
        }
        else
        {
            Serial.printf("Written only: %d / %d bytes\n", written, updateSize);
        }
        if (Update.end())
        {
            Serial.println("OTA done!");
            if (Update.isFinished())
            {
                Serial.println("Update installed successfully, rebooting...");
                return true;
            }
            else
            {
                Serial.println("Update failed.");
            }
        }
        else
        {
            Serial.printf("Error occured during install: %s\n", Update.errorString());
        }
    }
    else
    {
        Serial.println("Not enough space to perform update.");
    }
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, LOW);

    return false;
}

bool attemptToUpdateSelf(const String &fileName)
{
    File updateFile = LittleFS.open(fileName);
    if (updateFile)
    {
        size_t updateSize = updateFile.size();
        bool updated = false;
        if (updateSize > 0)
        {
            Serial.println("Attempting update...");

            updated = performUpdate(updateFile, updateSize);
        }
        else
        {
            Serial.println("Empty file, update aborted.");
        }
        updateFile.close();
        LittleFS.remove(fileName);

        return updated;
    }
    else
    {
        Serial.printf("Update file not found: %s\n", fileName);
    }
    return false;
}

bool performTeensyUpdate(Stream &updateSource, size_t updateSize)
{
    priorityMessageInProgress = true;
    Serial.println("Notifying Teensy of incoming priority message.");
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, HIGH);
    delay(50);
    TEENSY_SERIAL.println("FIRMWARE");
    delay(100);
    uint32_t transfered = 0;
    uint32_t percent = 0;
    if (!updateSource.available())
    {
        return false;
    }
    while (updateSource.available())
    {
        TEENSY_SERIAL.write(updateSource.read());
        if (int(100 * transfered / updateSize) > percent)
        {
            Serial.printf("Transfering firmware... %d%%\n", percent);
            percent = int(100 * transfered / updateSize);
        }
        transfered++;
        delayMicroseconds(20);
        if (TEENSY_SERIAL.available())
        {
            String teensyString = TEENSY_SERIAL.readString();
            Serial.printf("Teensy status: %s\n", teensyString);
        }
    }
    return true;
}

bool attemptToUpdateTeensy(const String &fileName)
{
    File updateFile = LittleFS.open(fileName);
    if (updateFile)
    {
        size_t updateSize = updateFile.size();
        bool updated = false;
        if (updateSize > 0)
        {
            Serial.println("Attempting to update Teensy...");

            updated = performTeensyUpdate(updateFile, updateSize);
        }
        else
        {
            Serial.println("Empty file, update aborted.");
        }
        updateFile.close();
        if (updated)
        {
            LittleFS.remove(fileName);
            Serial.println("Teensy firmware update sent.");
        }
        return updated;
    }
    else
    {
        Serial.printf("Update file not found: %s\n", fileName);
    }
    return false;
}
