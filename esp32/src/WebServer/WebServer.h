#ifndef _MY_WEBSERVER_H
#define _MY_WEBSERVER_H

#include <ESPAsyncWebServer.h>

extern AsyncWebServer *webServer;

extern bool uploadingFile;

void onFile(AsyncWebServerRequest *request);

void onUpdateNetworkConfig(AsyncWebServerRequest *request);

void onUpdateMotorConfig(AsyncWebServerRequest *request);

void onFileUpload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final);

void startWebServer();

String networkForm(uint16_t i, String initialSSID, String initialPassword);

String numberForm(String label, String name, String id, int32_t minValue, int32_t maxValue, int32_t initialValue);

String numberForm(String label, String name, String id, int32_t minValue, uint32_t maxValue, uint32_t initialValue);

String numberForm(String label, String name, String id, float minValue, float maxValue, float increment, float initialValue);

String humanReadableSize(const size_t bytes);

String listFiles(bool ishtml);

String teensyProcessor(const String &var);

String firmwareProcessor(const String &var);

#endif