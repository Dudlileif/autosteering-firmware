#include "WebServer.h"
#include "web_pages/web_pages.h"

#include <LittleFS.h>

#include "Network/Network.h"
#include "TeensyComms/TeensyComms.h"
#include "OTAUpdate/OTAUpdate.h"

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
          <input type="number" min=%MIN% max=%MAX% step="%INCREMENT%" name="%NAME%" id="%ID%" value="%VALUE%">
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

String networkProcessor(const String &var)
{
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
        else if (fileName.endsWith(".hex") && foundfile.size() > 0)
        {
          returnText += "<td><button onclick=\"installDownloadDeleteButton(\'" + fileName + "\', \'install\')\">Install on Teensy</button>";
        }
        returnText += "<td><button onclick=\"installDownloadDeleteButton(\'" + fileName + "\', \'download\')\">Download</button>";
        returnText += "<td><button onclick=\"installDownloadDeleteButton(\'" + fileName + "\', \'delete\')\">Delete</button></tr>";
      }
      else
      {
        returnText += "File: " + String(foundfile.name()) + " Size: " + humanReadableSize(foundfile.size()) + "\n";
      }
    }
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
  if (var == "FREE_STORAGE")
  {
    return humanReadableSize(LittleFS.totalBytes() - LittleFS.usedBytes());
  }
  if (var == "USED_STORAGE")
  {
    return humanReadableSize(LittleFS.usedBytes());
  }
  if (var == "TOTAL_STORAGE")
  {
    return humanReadableSize(LittleFS.totalBytes());
  }
  if (var == "ESP_FIRMWARE_VERSION")
  {
    return FIRMWARE_VERSION;
  }
  if (var == "TEENSY_FIRMWARE_DATE")
  {
    return teensyFirmwareVersion;
  }
  return String();
}

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
      if (key == "en_pwm_mode" || key == "pwm_autoscale" || key == "pwm_autograd" || key == "sfilt" || key == "sg_stop" || key == "chm" || key == "vhighfs" || key == "vhighchm")
      {
        form += checkboxForm(key, key, key, bool(kv.value()), "toggleCheckbox", motorConfig.getDescription(key));
      }
      else if (key == "hold_multiplier")
      {
        form += numberForm(key, key, key, 0.0, 1.0, 0.01, float(kv.value()), motorConfig.getDescription(key));
      }
      else if (key == "CAL_ROT")
      {
        form += numberForm(key, key, key, 0.0, 100.0, 0.1, float(kv.value()), motorConfig.getDescription(key));
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
        form += numberForm(key, key, key, 0, pow(2, 15) - 1, int16_t(kv.value()), motorConfig.getDescription(key));
      }
      else if (key == "TZEROWAIT")
      {
        form += numberForm(key, key, key, 0, pow(2, 16) - 1, uint16_t(kv.value()), motorConfig.getDescription(key));
      }
      else if (key == "VSTOP" || key == "VSTART")
      {
        form += numberForm(key, key, key, 0, pow(2, 18) - 1, uint16_t(kv.value()), motorConfig.getDescription(key));
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
  if (var == "TEENSY_CRASH_REPORT")
  {
    if (teensyCrashReport.isEmpty())
    {
      return String("No crash report found.");
    }
    return teensyCrashReport;
  }
  if (var == "ESP_UPTIME")
  {
    return uptimeMsToString(millis());
  }
  if (var == "TEENSY_UPTIME")
  {
    return uptimeMsToString(teensyUptimeMs);
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
      request->send(400, "text/plain", "ERROR: file does not exist");
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

void onUpdateNetworkConfig(AsyncWebServerRequest *request)
{
  for (int i = 0; i < 5; i++)
  {
    char ssid[16];
    char password[16];

    sprintf(ssid, "ssid_%d", i);
    sprintf(password, "password_%d", i);

    if (request->hasParam(ssid))
    {
      strcpy(wifiConfig.ssid[i], request->getParam(ssid)->value().c_str());
    }
    if (request->hasParam(password))
    {

      strcpy(wifiConfig.password[i], request->getParam(password)->value().c_str());
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

  wifiConfig.save(&LittleFS);
  Serial.printf("Saved WiFi config json:\n\t");
  wifiConfig.printToStreamPretty(&Serial);
  Serial.println();
  request->redirect("/network");
}

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
  if (request->hasParam("CAL_ROT"))
  {
    motorConfig.CAL_ROT = constrain(request->getParam("CAL_ROT")->value().toFloat(), 0.0, 100);
  }
  if (request->hasParam("was_min"))
  {
    motorConfig.was_min = constrain(request->getParam("was_min")->value().toInt(), 0, pow(2, 15) - 1);
  }
  if (request->hasParam("was_center"))
  {
    motorConfig.was_center = constrain(request->getParam("was_center")->value().toInt(), 0, pow(2, 15) - 1);
  }
  if (request->hasParam("was_max"))
  {
    motorConfig.was_max = constrain(request->getParam("was_max")->value().toInt(), 0, pow(2, 15) - 1);
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
  request->redirect("/motor");
}

void onFileUpload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
{
  uploadingFile = true;
  if (!index)
  {
    Serial.printf("Uploading to file: %s\n", "/" + filename);
    request->_tempFile = LittleFS.open("/" + filename, FILE_WRITE);
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
  if (var == "IP_ADDRESS")
  {
    return getIPAddress().toString();
  }
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

  webServer->on("/motor", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "text/html", motor_html, motorProcessor); });

  webServer->on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
                { 
                  bool success = getTeensyUptime(true);
                request->send_P(200, "text/html", status_html, statusProcessor); });

  webServer->on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->redirect("/"); 
                ESP.restart(); });

  webServer->on("/reboot_teensy", HTTP_GET, [](AsyncWebServerRequest *request)
                { rebootTeensy(); 
                request->redirect("/"); });

  webServer->on("/refresh_teensy_crash_report", HTTP_GET, [](AsyncWebServerRequest *request)
                { 
                  bool success = getTeensyCrashReport(true);
    request->redirect("/status"); });

  webServer->on("/refresh_teensy_uptime", HTTP_GET, [](AsyncWebServerRequest *request)
                { 
                  bool success = getTeensyUptime(true);
    request->redirect("/status"); });

  webServer->on("/refresh_teensy_version", HTTP_GET, [](AsyncWebServerRequest *request)
                { 
                  bool success = getTeensyFirmwareVersion(true);
    request->redirect("/firmware"); });

  webServer->on("/update_network_config", HTTP_GET, onUpdateNetworkConfig);

  webServer->on("/update_motor_config", HTTP_GET, onUpdateMotorConfig);

  webServer->on("/list_files", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(200, "text/plain", listFiles(true)); });

  webServer->on("/file", HTTP_GET, onFile);

  webServer->onFileUpload(onFileUpload);

  webServer->onNotFound([](AsyncWebServerRequest *request)
                        { request->send(404, "text/plain", "Not found"); });

  events->onConnect([](AsyncEventSourceClient *client)
                    {  
                      if(client->lastId()){
    Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
  }
  client->send("hello!", NULL, millis(), 1000); });

  webServer->begin();
}
