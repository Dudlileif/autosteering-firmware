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

#ifndef _MY_WEBSERVER_H
#define _MY_WEBSERVER_H

#include <ESPAsyncWebServer.h>

extern AsyncWebServer *webServer;

extern AsyncEventSource *events;

extern bool uploadingFile;

void onFile(AsyncWebServerRequest *request);

void onUpdateNetworkConfig(AsyncWebServerRequest *request);
void onUpdateNetworkConfigLocal(AsyncWebServerRequest *request);
void onUpdateNetworkConfigRemote(AsyncWebServerRequest *request);

#ifdef AUTOSTEERING_BRIDGE
void onUpdateMotorConfig(AsyncWebServerRequest *request);
void onUpdateMotorConfigLocal(AsyncWebServerRequest *request);
void onUpdateMotorConfigRemote(AsyncWebServerRequest *request);
#endif

void onFileUpload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final);

void startWebServer();

String networkForm(uint16_t i, String initialSSID, String initialPassword);

String numberForm(String label, String name, String id, int32_t minValue, int32_t maxValue, int32_t initialValue);
String numberForm(String label, String name, String id, int32_t minValue, int32_t maxValue, int32_t initialValue, String description);

String numberForm(String label, String name, String id, int32_t minValue, uint32_t maxValue, uint32_t initialValue);
String numberForm(String label, String name, String id, int32_t minValue, uint32_t maxValue, uint32_t initialValue, String description);

String numberForm(String label, String name, String id, float minValue, float maxValue, float increment, float initialValue);
String numberForm(String label, String name, String id, float minValue, float maxValue, float increment, float initialValue, String description);

String humanReadableSize(const size_t bytes);

String listFiles(bool ishtml);

String uptimeMsToString(long uptimeMs);

String statusProcessor(const String &var);

String firmwareProcessor(const String &var);

#endif