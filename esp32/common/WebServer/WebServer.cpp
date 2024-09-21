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

#include "WebServer.h"

#include <LittleFS.h>

#include "../Network/Network.h"
#include "../OTAUpdate/OTAUpdate.h"

AsyncWebServer *webServer;

AsyncEventSource *events;

bool uploadingFile = false;

String networkForm(uint16_t i, String initialSSID, String initialPassword)
{
  String network = R"(
    <tr>
      <td>
        <h4>Network %I+1%</h4>
      </td>
      <td>
        <input type="text" maxlength=32 name="ssid_%I%" id="ssid_%I%" value="%SSID%">    
      </td>
      <td>
        <input type="password" maxlength=32 name="password_%I%" id="password_%I%" value="%PASSWORD%">
      </td>    
    </tr>
    )";
  network.replace("%I%", String(i));
  network.replace("%I+1%", String(i + 1));
  network.replace("%SSID%", initialSSID);
  network.replace("%PASSWORD%", initialPassword);

  return network;
}

String numberForm(String label, String name, String id, int32_t minValue, int32_t maxValue, int32_t initialValue)
{
  return numberForm(label, name, id, minValue, maxValue, initialValue, "");
}

String numberForm(String label, String name, String id, int32_t minValue, int32_t maxValue, int32_t initialValue, String description)
{
  String form = R"(
    <tr>
        <td>
          <h4>%LABEL%</h4>
        </td>
        <td>
          <input type="number" min=%MIN% max=%MAX% name="%NAME%" id="%ID%" value="%VALUE%">
        </td>
    )";
  form.replace("%LABEL%", label);
  form.replace("%NAME%", name);
  form.replace("%ID%", id);
  form.replace("%MIN%", String(minValue));
  form.replace("%MAX%", String(maxValue));
  form.replace("%VALUE%", String(initialValue));

  if (!description.isEmpty())
  {
    form += R"(
      <td>
        %DESCRIPTION%
      </td>
    )";
    form.replace("%DESCRIPTION%", description);
  }

  form += "</tr>";
  return form;
}

String numberForm(String label, String name, String id, int32_t minValue, uint32_t maxValue, uint32_t initialValue)
{
  return numberForm(label, name, id, minValue, maxValue, initialValue, "");
}

String numberForm(String label, String name, String id, int32_t minValue, uint32_t maxValue, uint32_t initialValue, String description)

{
  String form = R"(
      <tr>
        <td>
          <h4>%LABEL%</h4>
        </td>
        <td>
          <input type="number" min=%MIN% max=%MAX% name="%NAME%" id="%ID%" value="%VALUE%">
        </td>
    )";
  form.replace("%LABEL%", label);
  form.replace("%NAME%", name);
  form.replace("%ID%", id);
  form.replace("%MIN%", String(minValue));
  form.replace("%MAX%", String(maxValue));
  form.replace("%VALUE%", String(initialValue));

  if (!description.isEmpty())
  {
    form += R"(
      <td>
        %DESCRIPTION%
      </td>
    )";
    form.replace("%DESCRIPTION%", description);
  }

  form += "</tr>";
  return form;
}

String numberForm(String label, String name, String id, float minValue, float maxValue, float increment, float initialValue)
{
  return numberForm(label, name, id, minValue, maxValue, increment, initialValue, "");
}

String numberForm(String label, String name, String id, float minValue, float maxValue, float increment, float initialValue, String description)
{
  String form = R"(
      <tr>
        <td>
          <h4>%LABEL%</h4>
        </td>
        <td>
          <input type="number" min=%MIN% max=%MAX% step="any" name="%NAME%" id="%ID%" value="%VALUE%">
        </td>
    )";

  form.replace("%LABEL%", label);
  form.replace("%NAME%", name);
  form.replace("%ID%", id);
  form.replace("%MIN%", String(minValue));
  form.replace("%MAX%", String(maxValue));
  form.replace("%INCREMENT%", String(increment));
  form.replace("%VALUE%", String(initialValue));

  if (!description.isEmpty())
  {
    form += R"(
      <td>
        %DESCRIPTION%
      </td>
    )";
    form.replace("%DESCRIPTION%", description);
  }

  form += "</tr>";

  return form;
}

// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
String humanReadableSize(const size_t bytes)
{
  if (bytes < 1024)
    return String(bytes) + " B";
  else if (bytes < (1024 * 1024))
    return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024))
    return String(bytes / 1024.0 / 1024.0) + " MB";
  else
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

String titleReplacer()
{
#ifdef BASE_STATION_RELAY
  return String("Base Station");
#endif
#ifdef AUTOSTEERING_BRIDGE
  return String("Tractor Autosteering");
#endif
#ifdef AUTOSTEERING_REMOTE_CONTROL
  return String("Tractor Remote Control");
#endif
#ifdef STEPPER_MOTOR_TESTING
  return String("Stepper Motor Testing");
#endif
  return String("Firmware type missing!");
}

String networkProcessor(const String &var)
{
  if (var == "TITLE")
  {
    return titleReplacer();
  }
  if (var == "NETWORK_CONFIG_PLACEHOLDER")
  {
    String form;

    for (int i = 0; i < 5; i++)
    {
      form += networkForm(i, wifiConfig.ssid[i], wifiConfig.password[i]);
    }

    return form;
  }
  if (var == "AP_CONFIG_PLACEHOLDER")
  {
    String form = R"(
      <tr>
        <td>
          <input type="text" maxlength=32 name="ssid_ap" id="ssid_ap" value="%SSID%">
        </td>
        <td>
          <input type="password" maxlength=32 name="password_ap" id="password_ap" value="%PASSWORD%">
        </td>
        <td>
          <input type="submit" value="Submit">
        </td>
      </tr> 
      )";
    form.replace("%SSID%", wifiConfig.apSSID);
    form.replace("%PASSWORD%", wifiConfig.apPassword);

    return form;
  }
  if (var == "HOSTNAME_VALUE")
  {
    return wifiConfig.hostname;
  }
  if (var == "START_IN_AP_MODE_VALUE")
  {
    return wifiConfig.startInAPMode ? "checked" : "";
  }
#ifdef BASE_STATION_RELAY
  if (var == "BASE_STATION_RELAY")
  {
    String form = R"(
      <h3>RTKBase station address</h3>
      <form action="/update_network_config_local">
        <table>
          <tr>
            <td>
                <h4>Address</h4>
            </td>
            <td>
              <input type="text" maxlength=32 name="rtk_base_station_address" id="rtk_base_station_address" value="%VALUE%">
            </td>
            <td>
              <input type="submit" value="Submit">
            </td>
          </tr>
        </table>
      </form>
      <br>
      )";
    form.replace("%VALUE%", wifiConfig.rtkBaseStationAddress);
    return form;
  }
#endif
  if (var == "NETWORK_PORTS_PLACEHOLDER")
  {
    String form;

    form += numberForm("TCP receive", "tcp_receive_port", "tcp_receive_port", 0, 65535, wifiConfig.tcpReceivePort);
    form += numberForm("TCP send", "tcp_send_port", "tcp_send_port", 0, 65535, wifiConfig.tcpSendPort);
    form += numberForm("UDP receive", "udp_receive_port", "udp_receive_port", 0, 65535, wifiConfig.udpReceivePort);
    form += numberForm("UDP send", "udp_send_port", "udp_send_port", 0, 65535, wifiConfig.udpSendPort);

    return form;
  }

  return String();
}

// list all of the files, if ishtml=true, return html rather than simple text
String listFiles(bool ishtml)
{
  String returnText = "";
  File root = LittleFS.open("/");
  File foundfile = root.openNextFile();
  {
    returnText += "<table><tr><th align='left'>Name</th><th align='left'>Size</th><th></th><th></th></tr>";
  }
  while (foundfile)
  {
    String fileName = String(foundfile.name());
    if (!fileName.endsWith(".bak") && !fileName.endsWith(".dat"))
    {
      if (ishtml)
      {
        returnText += "<tr align='left'><td>" + fileName + "</td><td>" + humanReadableSize(foundfile.size()) + "</td>";
        if (fileName.endsWith(".bin") && foundfile.size() > 0)
        {
          returnText += "<td><button onclick=\"installDownloadDeleteButton(\'" + fileName + "\', \'install\')\">Install on ESP</button>";
        }
#ifdef AUTOSTEERING_BRIDGE
        else if (fileName.endsWith(".hex") && foundfile.size() > 0)
        {
          returnText += "<td><button onclick=\"installDownloadDeleteButton(\'" + fileName + "\', \'install\')\">Install on Teensy</button>";
        }
#endif
        returnText += "<td><button onclick=\"installDownloadDeleteButton(\'" + fileName + "\', \'download\')\">Download</button>";
        returnText += "<td><button onclick=\"installDownloadDeleteButton(\'" + fileName + "\', \'delete\')\">Delete</button></tr>";
      }
      else
      {
        returnText += "File: " + String(foundfile.name()) + " Size: " + humanReadableSize(foundfile.size()) + "\n";
      }
    }
    foundfile.close();
    foundfile = root.openNextFile();
  }
  if (ishtml)
  {
    returnText += "</table>";
  }
  root.close();
  foundfile.close();
  return returnText;
}

String firmwareProcessor(const String &var)
{
  if (var == "TITLE")
  {
    return titleReplacer();
  }
  if (var == "ESP_FIRMWARE_VERSION")
  {
    return String(FIRMWARE_TYPE) + String("_") + String(VERSION);
  }

  return String();
}

String checkboxForm(String label, String name, String id, bool checked, String function, String description)
{
  String form = R"=====(
    <tr>
        <td>
          <h4>%LABEL%</h4>
        </td>
        <td>
          <input type="checkbox" name="%NAME%" id="%ID%" %CHECKED% onClick="%FUNCTION%(this)">
        </td>
    )=====";
  form.replace("%LABEL%", label);
  form.replace("%NAME%", name);
  form.replace("%ID%", id);
  form.replace("%CHECKED%", checked ? "checked" : "");
  form.replace("%FUNCTION%", function);

  if (!description.isEmpty())
  {
    form += R"(
      <td>
        %DESCRIPTION%
      </td>
    )";
    form.replace("%DESCRIPTION%", description);
  }

  form += "</tr>";
  return form;
}

String uptimeMsToString(long uptimeMs)
{
  uint32_t uptimeSeconds = uptimeMs / 1000;
  uint8_t days = uptimeSeconds / (86400);
  uint8_t hours = (uptimeSeconds - (days * 86400)) / 3600;
  uint8_t minutes = floor((uptimeSeconds - (days * 86400) - (hours * 3600)) / 60);
  uint8_t seconds = uptimeSeconds - days * 86400 - hours * 3600 - minutes * 60;
  char buffer[100];
  if (days > 0)
  {
    sprintf(buffer, "%d d %d h %d m %d s", days, hours, minutes, seconds);
  }
  else if (hours > 0)
  {
    sprintf(buffer, "%d h %d m %d s", hours, minutes, seconds);
  }
  else if (minutes > 0)
  {
    sprintf(buffer, "%d m %d s", minutes, seconds);
  }
  else
  {
    sprintf(buffer, "%d s", seconds);
  }
  return String(buffer);
}

String statusProcessor(const String &var)
{
  if (var == "TITLE")
  {
    return titleReplacer();
  }
  if (var == "ESP_UPTIME")
  {
    return uptimeMsToString(millis());
  }
  return String();
}

void onFile(AsyncWebServerRequest *request)
{
  if (request->hasParam("name") && request->hasParam("action"))
  {
    const char *fileAction = request->getParam("action")->value().c_str();

    String fileName = String("/");
    fileName += request->getParam("name")->value();

    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url() + "?name=" + fileName + "&action=" + String(fileAction);

    if (!LittleFS.exists(fileName))
    {
      Serial.println(logmessage + " ERROR: file does not exist");
      request->send(400, "text/plain", String("ERROR: file does not exist: " + fileName));
    }
    else
    {
      Serial.println(logmessage + " file exists");
      if (strcmp(fileAction, "install") == 0)
      {
        doUpdate = true;
        updateFileName = fileName;
        logmessage += " attempting to install.";
        request->send(200, "text/plain", "Attempting to install...");
      }
      else if (strcmp(fileAction, "download") == 0)
      {
        logmessage += " downloaded";
        request->send(LittleFS, fileName, "application/octet-stream");
      }
      else if (strcmp(fileAction, "delete") == 0)
      {
        logmessage += " deleted";
        LittleFS.remove(fileName);
        request->send(200, "text/plain", "Deleted File: " + fileName);
      }
      else
      {
        logmessage += " ERROR: invalid action param supplied";
        request->send(400, "text/plain", "ERROR: invalid action param supplied");
      }
      Serial.println(logmessage);
    }
  }
  else
  {
    request->send(400, "text/plain", "ERROR: name and action params required");
  }
}

void onDeleteUpdateFiles(AsyncWebServerRequest *request)
{
  File root = LittleFS.open("/");
  File foundFile = root.openNextFile();

  String toRemove[10];

  while (foundFile)
  {
    String fileName = "/" + String(foundFile.name());
    foundFile.close();
    if (fileName.endsWith(".bin") || fileName.endsWith(".hex"))
    {
      if (LittleFS.exists(fileName))
      {
        if (LittleFS.remove(fileName))
        {
          Serial.println("Deleted file: " + fileName);
        }
      }
    }
    foundFile = root.openNextFile();
  }
  root.close();
  request->redirect("/firmware");
}

void onUpdateNetworkConfig(AsyncWebServerRequest *request)
{
  int numEmptySSIDs = 0;
  for (int i = 0; i < 5; i++)
  {
    char ssid[16];
    char password[16];

    sprintf(ssid, "ssid_%d", i);
    sprintf(password, "password_%d", i);

    bool ssidIsEmpty = false;

    if (request->hasParam(ssid))
    {
      String value = request->getParam(ssid)->value();
      ssidIsEmpty = value.isEmpty();
      if (ssidIsEmpty)
      {
        numEmptySSIDs += 1;
      }
      strlcpy(wifiConfig.ssid[ssidIsEmpty ? i : i - numEmptySSIDs], ssidIsEmpty ? "" : value.c_str(), 32);
    }
    if (request->hasParam(password))
    {
      strlcpy(wifiConfig.password[ssidIsEmpty ? i : i - numEmptySSIDs], ssidIsEmpty ? "" : request->getParam(password)->value().c_str(), 32);
    }
    if (!ssidIsEmpty)
    {
      wifiMulti.addAP(wifiConfig.ssid[i], strlen(wifiConfig.password[i]) > 0 ? wifiConfig.password[i] : NULL);
    }
  }
  if (request->hasParam("hostname"))
  {
    strlcpy(wifiConfig.hostname, request->getParam("hostname")->value().c_str(), 32);
  }
  if (request->hasParam("ssid_ap"))
  {
    strlcpy(wifiConfig.apSSID, request->getParam("ssid_ap")->value().c_str(), 32);
  }
  if (request->hasParam("password_ap"))
  {
    strlcpy(wifiConfig.apPassword, request->getParam("password_ap")->value().c_str(), 32);
  }
  if (request->hasParam("start_in_ap_mode"))
  {
    String value = request->getParam("start_in_ap_mode")->value();
    if (value == "on")
    {
      wifiConfig.startInAPMode = true;
    }
    else if (value == "off")
    {
      wifiConfig.startInAPMode = false;
    }
  }
  if (request->hasParam("tcp_receive_port"))
  {
    wifiConfig.tcpReceivePort = request->getParam("tcp_receive_port")->value().toInt();
  }
  if (request->hasParam("tcp_send_port"))
  {
    wifiConfig.tcpSendPort = request->getParam("tcp_send_port")->value().toInt();
  }
  if (request->hasParam("udp_receive_port"))
  {
    wifiConfig.udpReceivePort = request->getParam("udp_receive_port")->value().toInt();
  }
  if (request->hasParam("udp_send_port"))
  {
    wifiConfig.udpSendPort = request->getParam("udp_send_port")->value().toInt();
  }
#ifdef BASE_STATION_RELAY
  if (request->hasParam("rtk_base_station_address"))
  {
    strlcpy(wifiConfig.rtkBaseStationAddress, request->getParam("rtk_base_station_address")->value().c_str(), 32);
  }
#endif

  wifiConfig.save(&LittleFS);
  Serial.printf("Saved WiFi config json:\n\t");
  wifiConfig.printToStreamPretty(&Serial);
  Serial.println();
}

void onUpdateNetworkConfigLocal(AsyncWebServerRequest *request)
{
  onUpdateNetworkConfig(request);
  request->redirect("/network");
}

void onUpdateNetworkConfigRemote(AsyncWebServerRequest *request)
{
  onUpdateNetworkConfig(request);
  request->send(200, "text/plain", "Network config updated.");
}

void onFoundNetworks(AsyncWebServerRequest *request)
{
  int foundNetworks = WiFi.scanComplete();
  JsonDocument doc;
  if (foundNetworks == -2)
  {
    doc["status"] = "Network scan failed";
    WiFi.scanNetworks(true);
  }
  else if (foundNetworks == -1)
  {
    doc["status"] = "Network scan running";
  }
  else
  {
    doc["status"] = "Network scan finished";
    JsonArray networks = doc["networks"].to<JsonArray>();

    for (int i = 0; i < foundNetworks; i++)
    {
      JsonDocument network;
      network["ssid"] = WiFi.SSID(i);
      network["rssi"] = WiFi.RSSI(i);
      network["auth"] = WiFi.encryptionType(i);
      network["auth_name"] = wifiAuthTypeToString(WiFi.encryptionType(i));
      networks.add(network);
    }
  }
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

#
void onFileUpload(AsyncWebServerRequest *request, const String &fileName, size_t index, uint8_t *data, size_t len, bool final)
{
  uploadingFile = true;
  if (!index)
  {
    Serial.printf("Uploading to file: /%s\n", fileName.c_str());
    request->_tempFile = LittleFS.open("/" + fileName, FILE_WRITE);
  }
  if (len)
  {
    request->_tempFile.write(data, len);
  }
  if (final)
  {
    request->_tempFile.close();
    Serial.println("Upload finished");
    request->redirect("/firmware");
    uploadingFile = false;
  }
}

String mainProcessor(const String &var)
{
  if (var == "TITLE")
  {
    return titleReplacer();
  }
  if (var == "IP_ADDRESS")
  {
    return getIPAddress().toString();
  }
#if defined(AUTOSTEERING_BRIDGE) || defined(STEPPER_MOTOR_TESTING)
  if (var == "MOTOR")
  {

    return String(R"(<tr>
                <td>
                    <form action="/motor">
                        <button type="submit">
                            <h3>Motor configuration</h3>
                        </button>
                    </form>
                    <br>
                </td>
            </tr>)");
  }
#endif
  return String();
}

void startWebServer()
{

  webServer = new AsyncWebServer(wifiConfig.apServerPort);

  events = new AsyncEventSource("/events");

  webServer->addHandler(events);

  webServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(200, "text/html", main_html, mainProcessor); });

  webServer->on("/network", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(200, "text/html", network_html, networkProcessor); });

  webServer->on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
                {request->send(200, "text/plain", "ESP32 rebooting");
                  ESP.restart(); });

  webServer->on("/refresh_esp_uptime", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(200, "text/plain", uptimeMsToString(millis()).c_str()); });

  webServer->on("/update_network_config", HTTP_GET, onUpdateNetworkConfigRemote);
  webServer->on("/update_network_config_local", HTTP_GET, onUpdateNetworkConfigLocal);

  webServer->on("/scan_networks", HTTP_GET, [](AsyncWebServerRequest *request)
                { WiFi.scanNetworks(true);
                  request->send(200, "text/plain", "Network scanning started."); });

  webServer->on("/found_networks", HTTP_GET, onFoundNetworks);

  webServer->on("/list_files", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(200, "text/plain", listFiles(true)); });

  webServer->on("/file", HTTP_GET, onFile);

  webServer->on("/delete_update_files", HTTP_GET, onDeleteUpdateFiles);

  webServer->on("/storage_sizes", HTTP_GET, [](AsyncWebServerRequest *request)
                { String response;
                  JsonDocument doc;
                  doc["free"] = LittleFS.totalBytes() - LittleFS.usedBytes();
                  doc["used"] = LittleFS.usedBytes();
                  doc["total"] =LittleFS.totalBytes();
                  serializeJson(doc, response);
                  request->send(200, "application/json", response); });

  webServer->onFileUpload(onFileUpload);

  webServer->onNotFound([](AsyncWebServerRequest *request)
                        { request->send(404, "text/plain", "Not found"); });

  events->onConnect([](AsyncEventSourceClient *client)
                    {  
                      if(client->lastId()){
                        Serial.printf("Client reconnected! Last message ID that it got was: %u\n", client->lastId());
                      }
                      client->send("Hello!", NULL, millis(), 1000); });

#if defined(AUTOSTEERING_REMOTE_CONTROL) || defined(BASE_STATION_RELAY) || defined(STEPPER_MOTOR_TESTING)
  webServer->on("/firmware", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(200, "text/html", firmware_html, firmwareProcessor); });

  webServer->on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(200, "text/html", status_html, statusProcessor); });
#ifndef STEPPER_MOTOR_TESTING
  webServer->begin();
#endif
#endif
}
