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

#include "TeensyWebServerAddtions.h"
#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <MotorConfig.h>
#include <LittleFS.h>
#include <OTAUpdate.h>
#include "../TeensyComms/TeensyComms.h"
#include "../TeensyOTAUpdateAdditions/TeensyOTAUpdateAdditions.h"

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

String firmwareWithTeensyProcessor(const String &var)
{
    if (var == "TITLE")
    {
        return titleReplacer();
    }
#ifdef AUTOSTEERING_BRIDGE
    if (var == "TEENSY_VERSION_SEGMENT")
    {
        return String(R"rawliteral(
      <br>
      Teensy: <span id="teensy_version">%TEENSY_FIRMWARE_VERSION%</span>
      <button onclick="getTeensyVersion()">Refresh Teensy version</button>)rawliteral");
    }
    if (var == "TEENSY_FIRMWARE_VERSION")
    {
        return teensyFirmwareVersion;
    }
#endif
    if (var == "ESP_FIRMWARE_VERSION")
    {
        return String(FIRMWARE_TYPE) + String("_") + String(VERSION);
    }

    return String();
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

String statusWithTeensyProcessor(const String &var)
{
    if (var == "TITLE")
    {
        return titleReplacer();
    }
    if (var == "ESP_UPTIME")
    {
        return uptimeMsToString(millis());
    }

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
    return String();
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

void addTeensyCallbacksToWebServer()
{
    webServer->on("/firmware", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send_P(200, "text/html", firmware_html, firmwareWithTeensyProcessor); });
    webServer->on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
    bool success = getTeensyUptime(true);
    request->send_P(200, "text/html", status_html, statusWithTeensyProcessor); });
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
    webServer->begin();
}
