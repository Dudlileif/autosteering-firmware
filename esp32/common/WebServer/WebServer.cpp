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
#include "web_pages/web_pages.h"

#include <LittleFS.h>

#include "../Network/Network.h"
#include "../OTAUpdate/OTAUpdate.h"

#ifdef AUTOSTEERING_BRIDGE
#include "../TeensyComms/TeensyComms.h"
#endif

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
  if (var == "TEENSY_VERSION_SEGMENT")
  {
#ifdef BASE_STATION_RELAY
    return String();
#endif
#ifdef AUTOSTEERING_BRIDGE
    return String(R"rawliteral(
      <br>
      Teensy: <span id="teensy_version">%TEENSY_FIRMWARE_VERSION%</span>
      <button onclick="getTeensyVersion()">Refresh Teensy version</button>)rawliteral");
#endif
  }
  if (var == "ESP_FIRMWARE_VERSION")
  {
    return String(FIRMWARE_TYPE) + String("_") + String(VERSION);
  }

#ifdef AUTOSTEERING_BRIDGE
  if (var == "TEENSY_FIRMWARE_VERSION")
  {
    return teensyFirmwareVersion;
  }
#endif

  return String();
}

#ifdef AUTOSTEERING_BRIDGE
String microStepsForm(uint16_t value)
{
  String entry = R"=====(<input type = "radio" name = "MICRO_STEPS" id = "%VALUE%" value = "%VALUE%" %CHECKED%>
      <label for="%VALUE%">%LABEL%</label>
)=====";

  entry.replace("%VALUE%", String(value));
  entry.replace("%CHECKED%", motorConfig.MICRO_STEPS == value ? "checked" : "");
  entry.replace("%LABEL%", value == 0 ? "FULLSTEP" : String(value));
  return entry;
}
#endif

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

#ifdef AUTOSTEERING_BRIDGE
String motorProcessor(const String &var)
{
  if (var == "MOTOR_CONFIG_PLACEHOLDER")
  {
    String form;
    JsonDocument document = motorConfig.json();
    for (JsonPair kv : document.as<JsonObject>())
    {
      String key = String(kv.key().c_str());
      // Filter boolean parameters
      if (key == "en_pwm_mode" || key == "pwm_autoscale" || key == "pwm_autograd" || key == "sfilt" || key == "sg_stop" || key == "chm" || key == "vhighfs" || key == "vhighchm" || key == "invertDirection")
      {
        form += checkboxForm(key, key, key, bool(kv.value()), "toggleCheckbox", motorConfig.getDescription(key));
      }
      else if (key == "hold_multiplier")
      {
        form += numberForm(key, key, key, 0.0, 1.0, 0.01, float(kv.value()), motorConfig.getDescription(key));
      }
      else if (key == "SGT")
      {
        form += numberForm(key, key, key, -64, 63, int8_t(kv.value()), motorConfig.getDescription(key));
      }
      else if (key == "freewheel" || key == "TBL")
      {
        form += numberForm(key, key, key, 0, 3, uint8_t(kv.value()), motorConfig.getDescription(key));
      }
      else if (key == "TOFF" || key == "HSTRT" || key == "HEND" || key == "IHOLDDELAY" || key == "semin" || key == "semax")
      {
        form += numberForm(key, key, key, 0, 15, uint8_t(kv.value()), motorConfig.getDescription(key));
      }
      else if (key == "DC_SG" || key == "TPOWERDOWN")
      {
        form += numberForm(key, key, key, 0, 255, uint8_t(kv.value()), motorConfig.getDescription(key));
      }
      else if (key == "VMAX_RPM" || key == "TPWMTHRS_RPM" || key == "TCOOLTHRS_RPM" || key == "THIGH_RPM" || key == "VDCMIN_RPM" || key == "AMAX_RPM_S_2")
      {
        form += numberForm(key, key, key, 0.0, 1000.0, 1.0, float(kv.value()), motorConfig.getDescription(key));
      }
      else if (key == "DC_TIME")
      {
        form += numberForm(key, key, key, 0, 1023, uint16_t(kv.value()), motorConfig.getDescription(key));
      }
      else if (key == "RMS_CURRENT")
      {
        form += numberForm(key, key, key, 0, 3000, uint16_t(kv.value()), motorConfig.getDescription(key));
      }
      else if (key == "was_min" || key == "was_center" || key == "was_max")
      {
        form += numberForm(key, key, key, 0, pow(2, 12) - 1, uint16_t(kv.value()), motorConfig.getDescription(key));
      }
      else if (key == "TZEROWAIT")
      {
        form += numberForm(key, key, key, 0, pow(2, 16) - 1, uint16_t(kv.value()), motorConfig.getDescription(key));
      }
      else if (key == "VSTOP" || key == "VSTART")
      {
        form += numberForm(key, key, key, 0, pow(2, 18) - 1, uint16_t(kv.value()), motorConfig.getDescription(key));
      }
      else if (key == "pid_P" || key == "pid_I" || key == "pid_D")
      {
        form += numberForm(key, key, key, 0.0, 100.0, 0.001, float(kv.value()), motorConfig.getDescription(key));
      }
      else if (key != "MICRO_STEPS" && key != "STEPS_PER_ROT")
      {
        String missing = R"(
          <br>
          Missing setup for variable: %VARIABLE%
          <br>
        )";
        missing.replace("%VARIABLE%", key);
        form += missing;
      }
    }
    return form;
  }
  if (var == "MICRO_STEPS_PLACEHOLDER")
  {
    String form;
    for (uint16_t value : {256, 128, 64, 32})
    {
      form += microStepsForm(value);
    }
    form += R"(<br>)";
    for (uint16_t value : {16, 8, 4, 2})
    {
      form += microStepsForm(value);
    }
    form += R"(<br>)";
    form += microStepsForm(0);
    return form;
  }
  if (var == "STEPS_PER_ROT_PLACEHOLDER")
  {
    String form;
    uint16_t values[2] = {400, 200};
    for (uint16_t value : values)
    {
      String entry = R"=====(<input type = "radio" name = "STEPS_PER_ROT" id = "%VALUE%" value = "%VALUE%" %CHECKED%>
      <label for="%VALUE%">%LABEL%</label>
)=====";
      entry.replace("%VALUE%", String(value));
      entry.replace("%CHECKED%", motorConfig.STEPS_PER_ROT == value ? "checked" : "");
      entry.replace("%LABEL%", String(value));

      form += entry;
    }
    return form;
  }
  return String();
}
#endif

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

#ifdef AUTOSTEERING_BRIDGE
  if (var == "TEENSY_CRASH_REPORT")
  {
    if (teensyCrashReport.isEmpty())
    {
      return String("No crash report found.");
    }
    return teensyCrashReport;
  }
  if (var == "TEENSY_UPTIME")
  {
    return uptimeMsToString(teensyUptimeMs);
  }
#endif

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
      strcpy(wifiConfig.ssid[ssidIsEmpty ? i : i - numEmptySSIDs], ssidIsEmpty ? "" : value.c_str());
    }
    if (request->hasParam(password))
    {
      strcpy(wifiConfig.password[ssidIsEmpty ? i : i - numEmptySSIDs], ssidIsEmpty ? "" : request->getParam(password)->value().c_str());
    }
    if (!ssidIsEmpty)
    {
      wifiMulti.addAP(wifiConfig.ssid[i], strlen(wifiConfig.password[i]) > 0 ? wifiConfig.password[i] : NULL);
    }
  }
  if (request->hasParam("hostname"))
  {
    strcpy(wifiConfig.hostname, request->getParam("hostname")->value().c_str());
  }
  if (request->hasParam("ssid_ap"))
  {
    strcpy(wifiConfig.apSSID, request->getParam("ssid_ap")->value().c_str());
  }
  if (request->hasParam("password_ap"))
  {
    strcpy(wifiConfig.apPassword, request->getParam("password_ap")->value().c_str());
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
    strcpy(wifiConfig.rtkBaseStationAddress, request->getParam("rtk_base_station_address")->value().c_str());
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

#ifdef AUTOSTEERING_BRIDGE
void onUpdateMotorConfig(AsyncWebServerRequest *request)
{
  Serial.println("Received motor config update");
  if (request->hasParam("AMAX_RPM_S_2"))
  {
    motorConfig.AMAX_RPM_S_2 = constrain(request->getParam("AMAX_RPM_S_2")->value().toFloat(), 0.0, 3597);
  }
  if (request->hasParam("VMAX_RPM"))
  {
    motorConfig.VMAX_RPM = constrain(request->getParam("VMAX_RPM")->value().toFloat(), 0.0, 1000);
  }
  if (request->hasParam("VSTOP"))
  {
    motorConfig.VSTOP = constrain(request->getParam("VSTOP")->value().toInt(), 0, pow(2, 18) - 1);
  }
  if (request->hasParam("VSTART"))
  {
    motorConfig.VSTART = constrain(request->getParam("VSTART")->value().toInt(), 0, pow(2, 18) - 1);
  }
  if (request->hasParam("TOFF"))
  {
    motorConfig.TOFF = constrain(request->getParam("TOFF")->value().toInt(), 0, 15);
  }
  if (request->hasParam("HSTRT"))
  {
    motorConfig.HSTRT = constrain(request->getParam("HSTRT")->value().toInt(), 0, 15);
  }
  if (request->hasParam("HEND"))
  {
    motorConfig.HEND = constrain(request->getParam("HEND")->value().toInt(), 0, 15);
  }
  if (request->hasParam("MICRO_STEPS"))
  {
    motorConfig.MICRO_STEPS = request->getParam("MICRO_STEPS")->value().toInt();
  }
  if (request->hasParam("STEPS_PER_ROT"))
  {
    motorConfig.STEPS_PER_ROT = request->getParam("STEPS_PER_ROT")->value().toInt();
  }
  if (request->hasParam("RMS_CURRENT"))
  {
    motorConfig.RMS_CURRENT = constrain(request->getParam("RMS_CURRENT")->value().toInt(), 0, 3000);
  }
  if (request->hasParam("hold_multiplier"))
  {
    motorConfig.hold_multiplier = constrain(request->getParam("hold_multiplier")->value().toFloat(), 0.0, 1.0);
  }
  if (request->hasParam("IHOLDDELAY"))
  {
    motorConfig.IHOLDDELAY = constrain(request->getParam("IHOLDDELAY")->value().toInt(), 0, 15);
  }
  if (request->hasParam("freewheel"))
  {
    motorConfig.freewheel = constrain(request->getParam("freewheel")->value().toInt(), 0, 3);
  }
  if (request->hasParam("TBL"))
  {
    motorConfig.TBL = constrain(request->getParam("TBL")->value().toInt(), 0, 3);
  }
  if (request->hasParam("TPOWERDOWN"))
  {
    motorConfig.TPOWERDOWN = constrain(request->getParam("TPOWERDOWN")->value().toInt(), 0, 255);
  }
  if (request->hasParam("TZEROWAIT"))
  {
    motorConfig.TZEROWAIT = constrain(request->getParam("TZEROWAIT")->value().toInt(), 0, pow(2, 16) - 1);
  }
  if (request->hasParam("en_pwm_mode"))
  {
    const String value = request->getParam("en_pwm_mode")->value();
    if (value.length() == 1)
    {
      motorConfig.en_pwm_mode = bool(value.toInt());
    }
  }
  if (request->hasParam("pwm_autoscale"))
  {
    const String value = request->getParam("pwm_autoscale")->value();
    if (value.length() == 1)
    {
      motorConfig.pwm_autoscale = bool(value.toInt());
    }
  }
  if (request->hasParam("invertDirection"))
  {
    const String value = request->getParam("invertDirection")->value();
    if (value.length() == 1)
    {
      motorConfig.invertDirection = bool(value.toInt());
    }
  }
  if (request->hasParam("pwm_autograd"))
  {
    const String value = request->getParam("pwm_autograd")->value();
    if (value.length() == 1)
    {
      motorConfig.pwm_autograd = bool(value.toInt());
    }
  }
  if (request->hasParam("SGT"))
  {
    motorConfig.SGT = constrain(request->getParam("SGT")->value().toInt(), -64, 63);
  }
  if (request->hasParam("sfilt"))
  {
    const String value = request->getParam("sfilt")->value();
    if (value.length() == 1)
    {
      motorConfig.sfilt = bool(value.toInt());
    }
  }
  if (request->hasParam("sg_stop"))
  {
    const String value = request->getParam("sg_stop")->value();
    if (value.length() == 1)
    {
      motorConfig.sg_stop = bool(value.toInt());
    }
  }
  if (request->hasParam("TPWMTHRS_RPM"))
  {
    motorConfig.TPWMTHRS_RPM = constrain(request->getParam("TPWMTHRS_RPM")->value().toFloat(), 0.0, 1000);
  }
  if (request->hasParam("chm"))
  {
    const String value = request->getParam("chm")->value();
    if (value.length() == 1)
    {
      motorConfig.chm = bool(value.toInt());
    }
  }
  if (request->hasParam("TCOOLTHRS_RPM"))
  {
    motorConfig.TCOOLTHRS_RPM = constrain(request->getParam("TCOOLTHRS_RPM")->value().toFloat(), 0.0, 1000);
  }
  if (request->hasParam("semin"))
  {
    motorConfig.semin = constrain(request->getParam("semin")->value().toInt(), 0, 15);
  }
  if (request->hasParam("semax"))
  {
    motorConfig.semax = constrain(request->getParam("semax")->value().toInt(), 0, 15);
  }
  if (request->hasParam("THIGH_RPM"))
  {
    motorConfig.THIGH_RPM = constrain(request->getParam("THIGH_RPM")->value().toFloat(), 0.0, 1000);
  }
  if (request->hasParam("vhighfs"))
  {
    const String value = request->getParam("vhighfs")->value();
    if (value.length() == 1)
    {
      motorConfig.vhighfs = bool(value.toInt());
    }
  }
  if (request->hasParam("vhighchm"))
  {
    const String value = request->getParam("vhighchm")->value();
    if (value.length() == 1)
    {
      motorConfig.vhighchm = bool(value.toInt());
    }
  }
  if (request->hasParam("VDCMIN_RPM"))
  {
    motorConfig.VDCMIN_RPM = constrain(request->getParam("VDCMIN_RPM")->value().toFloat(), 0.0, 1000);
  }
  if (request->hasParam("DC_TIME"))
  {
    motorConfig.DC_TIME = constrain(request->getParam("DC_TIME")->value().toInt(), 0, 1023);
  }
  if (request->hasParam("DC_SG"))
  {
    motorConfig.DC_SG = constrain(request->getParam("DC_SG")->value().toInt(), 0, 255);
  }
  if (request->hasParam("was_min"))
  {
    motorConfig.was_min = constrain(request->getParam("was_min")->value().toInt(), 0, pow(2, 12) - 1);
  }
  if (request->hasParam("was_center"))
  {
    motorConfig.was_center = constrain(request->getParam("was_center")->value().toInt(), 0, pow(2, 12) - 1);
  }
  if (request->hasParam("was_max"))
  {
    motorConfig.was_max = constrain(request->getParam("was_max")->value().toInt(), 0, pow(2, 12) - 1);
  }
  if (request->hasParam("pid_P"))
  {
    motorConfig.pid_P = constrain(request->getParam("pid_P")->value().toFloat(), 0, 100);
  }
  if (request->hasParam("pid_I"))
  {
    motorConfig.pid_I = constrain(request->getParam("pid_I")->value().toFloat(), 0, 100);
  }
  if (request->hasParam("pid_D"))
  {
    motorConfig.pid_D = constrain(request->getParam("pid_D")->value().toFloat(), 0, 100);
  }

  if (motorConfig.TOFF == 1 && motorConfig.TBL < 2)
  {
    motorConfig.TBL = 2;
  }

  motorConfig.save(&LittleFS);
  Serial.println("Saved Motor config json:\n\t");
  motorConfig.printToStreamPretty(&Serial);
  Serial.println();
  sendMotorConfig();
}

void onUpdateMotorConfigLocal(AsyncWebServerRequest *request)
{
  onUpdateMotorConfig(request);
  request->redirect("/motor");
}
void onUpdateMotorConfigRemote(AsyncWebServerRequest *request)
{
  onUpdateMotorConfig(request);
  request->send(200, "text/plain", "Motor config updated.");
}
#endif

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
#ifdef AUTOSTEERING_BRIDGE
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
                { request->send_P(200, "text/html", main_html, mainProcessor); });

  webServer->on("/firmware", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "text/html", firmware_html, firmwareProcessor); });

  webServer->on("/network", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "text/html", network_html, networkProcessor); });

  webServer->on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
                {
#ifdef AUTOSTEERING_BRIDGE  
                bool success = getTeensyUptime(true);
#endif
                request->send_P(200, "text/html", status_html, statusProcessor); });

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

#ifdef AUTOSTEERING_BRIDGE
  webServer->on("/motor", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "text/html", motor_html, motorProcessor); });

  webServer->on("/update_motor_config", HTTP_GET, onUpdateMotorConfigRemote);
  webServer->on("/update_motor_config_local", HTTP_GET, onUpdateMotorConfigLocal);

  webServer->on("/motor_config.json", HTTP_GET, [](AsyncWebServerRequest *request)
                { String motorConfigJson;
                  serializeJson(motorConfig.json(), motorConfigJson);
                  request->send(200, "application/json", motorConfigJson); });

  webServer->on("/refresh_teensy_version", HTTP_GET, [](AsyncWebServerRequest *request)
                { bool success = getTeensyFirmwareVersion(true);
                   request->send(200,"text/plain",teensyFirmwareVersion.c_str()); });

  webServer->on("/reboot_teensy", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(200, "text/plain", "Teensy rebooting");
                  rebootTeensy(); });

  webServer->on("/refresh_teensy_crash_report", HTTP_GET, [](AsyncWebServerRequest *request)
                { 
                  bool success = getTeensyCrashReport(true);
                    request->send(200,"text/plain",teensyCrashReport.c_str()); });

  webServer->on("/refresh_teensy_uptime", HTTP_GET, [](AsyncWebServerRequest *request)
                { bool success = getTeensyUptime(true);
                request->send(200,"text/plain",uptimeMsToString(teensyUptimeMs).c_str()); });
#endif

  webServer->onFileUpload(onFileUpload);

  webServer->onNotFound([](AsyncWebServerRequest *request)
                        { request->send(404, "text/plain", "Not found"); });

  events->onConnect([](AsyncEventSourceClient *client)
                    {  
                      if(client->lastId()){
                        Serial.printf("Client reconnected! Last message ID that it got was: %u\n", client->lastId());
                      }
                      client->send("Hello!", NULL, millis(), 1000); });

  webServer->begin();
}
