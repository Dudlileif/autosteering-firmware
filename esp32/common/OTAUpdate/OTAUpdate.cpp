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

#include "OTAUpdate.h"

#include <Update.h>
#include <LittleFS.h>

#include "../Config/Config.h"
#include "../WebServer/WebServer.h"

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
    priorityMessageInProgress = false;
#ifdef AUTOSTEERING_BRIDGE
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, LOW);
#endif
    doUpdate = false;
}

bool performUpdate(Stream &updateSource, size_t updateSize)
{
#ifdef AUTOSTEERING_BRIDGE
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, HIGH);
#endif
    if (Update.begin(updateSize))
    {
        Update.onProgress([](size_t prg, size_t sz)
                          {
                              uint8_t percent = int(100 * prg / sz);
                              char buffer[40];
                              sprintf(buffer, "ESP update progress: %d%%", percent);
                              Serial.printf("%s\r", String(buffer).c_str());
                              events->send(String(buffer).c_str(), "progress", millis()); });
        
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
                events->send("ESP update installed successfully, rebooting...", "progress", millis());
                return true;
            }
            else
            {
                Serial.println("Update failed.");
                events->send("ESP update failed", "progress", millis());
            }
        }
        else
        {
            char buffer[100];
            sprintf(buffer, "ESP update error occured during install:\n%s", Update.errorString());
            Serial.print(buffer);
            events->send(String(buffer).c_str(), "progress", millis());
        }
    }
    else
    {
        Serial.println("Not enough space to perform update.");
        events->send("ESP update not enough space to perform update.", "progress", millis());
    }
#ifdef AUTOSTEERING_BRIDGE
    digitalWrite(PRIORITY_MESSAGE_SIGNAL_PIN, LOW);
#endif
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
            events->send("ESP update empty file, update aborted.", "progress", millis());
        }
        updateFile.close();
        LittleFS.remove(fileName);

        return updated;
    }
    else
    {
        // char buffer[100];
        // sprintf(buffer, "ESP update file not found: %s\n", fileName.c_str());
        // Serial.println(buffer);
        // events->send(String(buffer).c_str(), "progress", millis());
    }
    return false;
}
