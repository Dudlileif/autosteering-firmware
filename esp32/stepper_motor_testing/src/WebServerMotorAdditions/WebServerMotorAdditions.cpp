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

#include "WebServerMotorAdditions.h"
#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include <MotorConfig.h>
#include "StepperMotor/StepperMotor.h"

String microStepsForm(uint16_t value)
{
    String entry = R"=====(<input type = "radio" name = "MICRO_STEPS" id = "micro_steps_%VALUE%" value = "%VALUE%" %CHECKED%>
      <label for="%VALUE%">%LABEL%</label>
)=====";

    entry.replace("%VALUE%", String(value));
    entry.replace("%CHECKED%", motorConfig.MICRO_STEPS == value ? "checked" : "");
    entry.replace("%LABEL%", value == 0 ? "FULLSTEP" : String(value));
    return entry;
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
            if (key == "en_pwm_mode" || key == "pwm_autoscale" || key == "pwm_autograd" || key == "sfilt" || key == "sg_stop" || key == "chm" || key == "vhighfs" || key == "vhighchm" || key == "reverseDirection")
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
            else if (key == "VMAX_RPM" || key == "TPWMTHRS_RPM" || key == "TCOOLTHRS_RPM" || key == "THIGH_RPM" || key == "VDCMIN_RPM" || key == "AMAX_RPM_S")
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
            String entry = R"=====(<input type = "radio" name = "STEPS_PER_ROT" id = "steps_per_rot_%VALUE%" value = "%VALUE%" %CHECKED%>
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

void onUpdateMotorConfig(AsyncWebServerRequest *request, bool post = false)
{
    Serial.println("Received motor config update");
    if (request->hasParam("AMAX_RPM_S", post))
    {
        motorConfig.AMAX_RPM_S = constrain(request->getParam("AMAX_RPM_S", post)->value().toFloat(), 0.0, 3597);
    }
    if (request->hasParam("VMAX_RPM", post))
    {
        motorConfig.VMAX_RPM = constrain(request->getParam("VMAX_RPM", post)->value().toFloat(), 0.0, 1000);
    }
    if (request->hasParam("VSTOP", post))
    {
        motorConfig.VSTOP = constrain(request->getParam("VSTOP", post)->value().toInt(), 0, pow(2, 18) - 1);
    }
    if (request->hasParam("VSTART", post))
    {
        motorConfig.VSTART = constrain(request->getParam("VSTART", post)->value().toInt(), 0, pow(2, 18) - 1);
    }
    if (request->hasParam("TOFF", post))
    {
        motorConfig.TOFF = constrain(request->getParam("TOFF", post)->value().toInt(), 0, 15);
    }
    if (request->hasParam("HSTRT", post))
    {
        motorConfig.HSTRT = constrain(request->getParam("HSTRT", post)->value().toInt(), 0, 15);
    }
    if (request->hasParam("HEND", post))
    {
        motorConfig.HEND = constrain(request->getParam("HEND", post)->value().toInt(), 0, 15);
    }
    if (request->hasParam("MICRO_STEPS", post))
    {
        motorConfig.MICRO_STEPS = request->getParam("MICRO_STEPS", post)->value().toInt();
    }
    if (request->hasParam("STEPS_PER_ROT", post))
    {
        motorConfig.STEPS_PER_ROT = request->getParam("STEPS_PER_ROT", post)->value().toInt();
    }
    if (request->hasParam("RMS_CURRENT", post))
    {
        motorConfig.RMS_CURRENT = constrain(request->getParam("RMS_CURRENT", post)->value().toInt(), 0, 3000);
    }
    if (request->hasParam("hold_multiplier", post))
    {
        motorConfig.hold_multiplier = constrain(request->getParam("hold_multiplier", post)->value().toFloat(), 0.0, 1.0);
    }
    if (request->hasParam("IHOLDDELAY", post))
    {
        motorConfig.IHOLDDELAY = constrain(request->getParam("IHOLDDELAY", post)->value().toInt(), 0, 15);
    }
    if (request->hasParam("freewheel", post))
    {
        motorConfig.freewheel = constrain(request->getParam("freewheel", post)->value().toInt(), 0, 3);
    }
    if (request->hasParam("TBL", post))
    {
        motorConfig.TBL = constrain(request->getParam("TBL", post)->value().toInt(), 0, 3);
    }
    if (request->hasParam("TPOWERDOWN", post))
    {
        motorConfig.TPOWERDOWN = constrain(request->getParam("TPOWERDOWN", post)->value().toInt(), 0, 255);
    }
    if (request->hasParam("TZEROWAIT", post))
    {
        motorConfig.TZEROWAIT = constrain(request->getParam("TZEROWAIT", post)->value().toInt(), 0, pow(2, 16) - 1);
    }
    if (request->hasParam("en_pwm_mode", post))
    {
        const String value = request->getParam("en_pwm_mode", post)->value();
        if (value.length() == 1)
        {
            motorConfig.en_pwm_mode = bool(value.toInt());
        }
    }
    if (request->hasParam("pwm_autoscale", post))
    {
        const String value = request->getParam("pwm_autoscale", post)->value();
        if (value.length() == 1)
        {
            motorConfig.pwm_autoscale = bool(value.toInt());
        }
    }
    if (request->hasParam("reverseDirection", post))
    {
        const String value = request->getParam("reverseDirection", post)->value();
        if (value.length() == 1)
        {
            motorConfig.reverseDirection = bool(value.toInt());
        }
    }
    if (request->hasParam("pwm_autograd", post))
    {
        const String value = request->getParam("pwm_autograd", post)->value();
        if (value.length() == 1)
        {
            motorConfig.pwm_autograd = bool(value.toInt());
        }
    }
    if (request->hasParam("SGT", post))
    {
        motorConfig.SGT = constrain(request->getParam("SGT", post)->value().toInt(), -64, 63);
    }
    if (request->hasParam("sfilt", post))
    {
        const String value = request->getParam("sfilt", post)->value();
        if (value.length() == 1)
        {
            motorConfig.sfilt = bool(value.toInt());
        }
    }
    if (request->hasParam("sg_stop", post))
    {
        const String value = request->getParam("sg_stop", post)->value();
        if (value.length() == 1)
        {
            motorConfig.sg_stop = bool(value.toInt());
        }
    }
    if (request->hasParam("TPWMTHRS_RPM", post))
    {
        motorConfig.TPWMTHRS_RPM = constrain(request->getParam("TPWMTHRS_RPM", post)->value().toFloat(), 0.0, 1000);
    }
    if (request->hasParam("chm", post))
    {
        const String value = request->getParam("chm", post)->value();
        if (value.length() == 1)
        {
            motorConfig.chm = bool(value.toInt());
        }
    }
    if (request->hasParam("TCOOLTHRS_RPM", post))
    {
        motorConfig.TCOOLTHRS_RPM = constrain(request->getParam("TCOOLTHRS_RPM", post)->value().toFloat(), 0.0, 1000);
    }
    if (request->hasParam("semin", post))
    {
        motorConfig.semin = constrain(request->getParam("semin", post)->value().toInt(), 0, 15);
    }
    if (request->hasParam("semax", post))
    {
        motorConfig.semax = constrain(request->getParam("semax", post)->value().toInt(), 0, 15);
    }
    if (request->hasParam("THIGH_RPM", post))
    {
        motorConfig.THIGH_RPM = constrain(request->getParam("THIGH_RPM", post)->value().toFloat(), 0.0, 1000);
    }
    if (request->hasParam("vhighfs", post))
    {
        const String value = request->getParam("vhighfs", post)->value();
        if (value.length() == 1)
        {
            motorConfig.vhighfs = bool(value.toInt());
        }
    }
    if (request->hasParam("vhighchm", post))
    {
        const String value = request->getParam("vhighchm", post)->value();
        if (value.length() == 1)
        {
            motorConfig.vhighchm = bool(value.toInt());
        }
    }
    if (request->hasParam("VDCMIN_RPM", post))
    {
        motorConfig.VDCMIN_RPM = constrain(request->getParam("VDCMIN_RPM", post)->value().toFloat(), 0.0, 1000);
    }
    if (request->hasParam("DC_TIME", post))
    {
        motorConfig.DC_TIME = constrain(request->getParam("DC_TIME", post)->value().toInt(), 0, 1023);
    }
    if (request->hasParam("DC_SG", post))
    {
        motorConfig.DC_SG = constrain(request->getParam("DC_SG", post)->value().toInt(), 0, 255);
    }
    if (request->hasParam("was_min", post))
    {
        motorConfig.wasMin = constrain(request->getParam("was_min", post)->value().toInt(), 0, pow(2, 12) - 1);
    }
    if (request->hasParam("was_center", post))
    {
        motorConfig.wasCenter = constrain(request->getParam("was_center", post)->value().toInt(), 0, pow(2, 12) - 1);
    }
    if (request->hasParam("was_max", post))
    {
        motorConfig.wasMax = constrain(request->getParam("was_max", post)->value().toInt(), 0, pow(2, 12) - 1);
    }
    if (request->hasParam("pid_P", post))
    {
        motorConfig.pidP = constrain(request->getParam("pid_P", post)->value().toFloat(), 0, 100);
    }
    if (request->hasParam("pid_I", post))
    {
        motorConfig.pidI = constrain(request->getParam("pid_I", post)->value().toFloat(), 0, 100);
    }
    if (request->hasParam("pid_D", post))
    {
        motorConfig.pidD = constrain(request->getParam("pid_D", post)->value().toFloat(), 0, 100);
    }

    if (motorConfig.TOFF == 1 && motorConfig.TBL < 2)
    {
        motorConfig.TBL = 2;
    }

    motorConfig.save(&LittleFS);
    Serial.println("Saved Motor config json:\n\t");
    motorConfig.printToStreamPretty(&Serial);
    updateStepperDriverConfig();
    Serial.println();

    char serialized[1024];
    serializeJson(motorConfig.json(), serialized);
    sendMessageToEvents(serialized, "motor_config");
}

void onUpdateMotorConfigLocal(AsyncWebServerRequest *request)
{
    onUpdateMotorConfig(request);
    request->redirect("/motor");
}
void onUpdateMotorConfigRemote(AsyncWebServerRequest *request)
{
    onUpdateMotorConfig(request, request->method() == HTTP_POST);
    request->send(200, "text/plain", "Motor config updated.");
}

void addMotorCallbacksToWebServer()
{
    webServer->on("/motor", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send(200, "text/html", motor_html, motorProcessor); });

    webServer->on("/update_motor_config", HTTP_GET, onUpdateMotorConfigRemote);
    webServer->on("/update_motor_config_post", HTTP_POST, onUpdateMotorConfigRemote);

    webServer->on("/update_motor_config_local", HTTP_GET, onUpdateMotorConfigLocal);

    webServer->on("/motor_config.json", HTTP_GET, [](AsyncWebServerRequest *request)
                  { String motorConfigJson;
                  serializeJson(motorConfig.json(), motorConfigJson);
                  request->send(200, "application/json", motorConfigJson); });

    webServer->on("/motor_control", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
        if (request->hasParam("data")){
            JsonDocument doc;
            deserializeJson(doc, request->getParam("data")->value());
            handleMotorControls(doc);
        }
        request->send(200); });

    webServer->begin();
}

uint16_t wasReadingPrev;
elapsedMicros webEventsDataElapsedTime;

void sendPeriodicDataToEvents()
{
    if (webEventsDataElapsedTime > 33333)
    {
        JsonDocument data = getSensorData();
        int size = measureJson(data);

        char serialized[512];
        serializeJson(data, serialized);
        sendMessageToEvents(serialized, "data");

        webEventsDataElapsedTime = 0;
    }
}

void sendMessageToEvents(char *message, const char *channel)
{
    events->send(message, channel, millis());
}

// Rounds a number to a certain number of decimals.
float roundToNumberOfDecimals(float value, int numDecimals)
{
    int sign = (value > 0) - (value < 0);

    return (int)(value * pow(10, numDecimals) - sign * 0.5) / (pow(10.0, numDecimals));
}

JsonDocument getSensorData()
{
    JsonDocument data;
    data["was"] = wasReading;
    data["was_min"] = motorConfig.wasMin;
    data["was_center"] = motorConfig.wasCenter;
    data["was_max"] = motorConfig.wasMax;
    data["motor_enabled"] = motorEnabled;
    data["motor_stalled"] = motorStalled;
    data["motor_rpm"] = roundToNumberOfDecimals(stepperRPMActual, 3);
    data["motor_sg"] = stepperStallguardResult;
    data["motor_cs"] = stepperCurrentScale;
    data["motor_pos"] = stepperPositionActual;
    data["was_target"] = wasTarget;
    data["was_change_rate"] = (int)wasChangeRate;
    data["was_error_over_time"] = (int)wasErrorOverTime;

    return data;
}
