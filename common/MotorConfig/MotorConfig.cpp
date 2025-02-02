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

#include "MotorConfig.h"
#include <ArduinoJson.h>
#include <elapsedMillis.h>

bool MotorConfig::load(Stream *stream)
{

    bool configReady = false;
    elapsedMillis waitStart;
    Serial.println("Waiting for motor config...");
    while (!configReady && waitStart < 2000)
    {
        if (stream->available())
        {
            Serial.println("Found motor config...");
            configReady = true;
            break;
        }
    }
    if (!configReady)
    {
        return false;
    }
    JsonDocument savedConfig;

    deserializeJson(savedConfig, *stream);

    Serial.println("Loading...");

    if (savedConfig.isNull())
    {
        Serial.println("Motor config is empty, loading default.");
        return false;
    }
    if (savedConfig["reverseDirection"].is<bool>())
    {
        reverseDirection = savedConfig["reverseDirection"];
    }
    if (savedConfig["AMAX_RPM_S"].is<float>())
    {
        AMAX_RPM_S = savedConfig["AMAX_RPM_S"];
    }
    if (savedConfig["DMAX_RPM_S"].is<float>())
    {
        DMAX_RPM_S = savedConfig["DMAX_RPM_S"];
    }
    if (savedConfig["VMAX_RPM"].is<float>())
    {
        VMAX_RPM = savedConfig["VMAX_RPM"];
    }
    if (savedConfig["VSTOP"].is<uint32_t>())
    {
        VSTOP = savedConfig["VSTOP"];
    }
    if (savedConfig["VSTART"].is<uint32_t>())
    {
        VSTART = savedConfig["VSTART"];
    }
    if (savedConfig["TOFF"].is<uint8_t>())
    {
        TOFF = savedConfig["TOFF"];
    }
    if (savedConfig["HSTRT"].is<uint8_t>())
    {
        HSTRT = savedConfig["HSTRT"];
    }
    if (savedConfig["HEND"].is<uint8_t>())
    {
        HEND = savedConfig["HEND"];
    }
    if (savedConfig["MICRO_STEPS"].is<uint16_t>())
    {
        MICRO_STEPS = savedConfig["MICRO_STEPS"];
    }
    if (savedConfig["STEPS_PER_ROT"].is<uint16_t>())
    {
        STEPS_PER_ROT = savedConfig["STEPS_PER_ROT"];
    }
    if (savedConfig["RMS_CURRENT"].is<uint16_t>())
    {
        RMS_CURRENT = savedConfig["RMS_CURRENT"];
    }
    if (savedConfig["hold_multiplier"].is<float>())
    {
        hold_multiplier = savedConfig["hold_multiplier"];
    }
    if (savedConfig["IHOLDDELAY"].is<uint8_t>())
    {
        IHOLDDELAY = savedConfig["IHOLDDELAY"];
    }
    if (savedConfig["freewheel"].is<uint8_t>())
    {
        freewheel = savedConfig["freewheel"];
    }
    if (savedConfig["TBL"].is<uint8_t>())
    {
        TBL = savedConfig["TBL"];
    }
    if (savedConfig["TPOWERDOWN"].is<uint8_t>())
    {
        TPOWERDOWN = savedConfig["TPOWERDOWN"];
    }
    if (savedConfig["TZEROWAIT"].is<uint16_t>())
    {
        TZEROWAIT = savedConfig["TZEROWAIT"];
    }
    if (savedConfig["en_pwm_mode"].is<bool>())
    {
        en_pwm_mode = savedConfig["en_pwm_mode"];
    }
    if (savedConfig["pwm_autoscale"].is<bool>())
    {
        pwm_autoscale = savedConfig["pwm_autoscale"];
    }
    if (savedConfig["pwm_autograd"].is<bool>())
    {
        pwm_autograd = savedConfig["pwm_autograd"];
    }
    if (savedConfig["SGT"].is<int8_t>())
    {
        SGT = savedConfig["SGT"];
    }
    if (savedConfig["sfilt"].is<bool>())
    {
        sfilt = savedConfig["sfilt"];
    }
    if (savedConfig["sg_stop"].is<bool>())
    {
        sg_stop = savedConfig["sg_stop"];
    }
    if (savedConfig["TPWMTHRS_RPM"].is<float>())
    {
        TPWMTHRS_RPM = savedConfig["TPWMTHRS_RPM"];
    }
    if (savedConfig["chm"].is<bool>())
    {
        chm = savedConfig["chm"];
    }
    if (savedConfig["TCOOLTHRS_RPM"].is<float>())
    {
        TCOOLTHRS_RPM = savedConfig["TCOOLTHRS_RPM"];
    }
    if (savedConfig["semin"].is<uint8_t>())
    {
        semin = savedConfig["semin"];
    }
    if (savedConfig["semax"].is<uint8_t>())
    {
        semax = savedConfig["semax"];
    }
    if (savedConfig["THIGH_RPM"].is<float>())
    {
        THIGH_RPM = savedConfig["THIGH_RPM"];
    }
    if (savedConfig["vhighfs"].is<bool>())
    {
        vhighfs = savedConfig["vhighfs"];
    }
    if (savedConfig["vhighchm"].is<bool>())
    {
        vhighchm = savedConfig["vhighchm"];
    }
    if (savedConfig["VDCMIN_RPM"].is<float>())
    {
        VDCMIN_RPM = savedConfig["VDCMIN_RPM"];
    }
    if (savedConfig["DC_TIME"].is<uint16_t>())
    {
        DC_TIME = savedConfig["DC_TIME"];
    }
    if (savedConfig["DC_SG"].is<uint8_t>())
    {
        DC_SG = savedConfig["DC_SG"];
    }
    if (savedConfig["pid_P"].is<float>())
    {
        pidP = savedConfig["pid_P"];
    }
    if (savedConfig["pid_I"].is<float>())
    {
        pidI = savedConfig["pid_I"];
    }
    if (savedConfig["pid_D"].is<float>())
    {
        pidD = savedConfig["pid_D"];
    }
    if (savedConfig["was_min"].is<uint16_t>())
    {
        wasMin = savedConfig["was_min"];
    }
    if (savedConfig["was_center"].is<uint16_t>())
    {
        wasCenter = savedConfig["was_center"];
    }
    if (savedConfig["was_max"].is<uint16_t>())
    {
        wasMax = savedConfig["was_max"];
    }

    Serial.println("Motor config loaded successfully.");

    return true;
}

bool MotorConfig::loadFromFile(FS *fs)
{
    bool configExists = fs->exists(MOTOR_CONFIG_FILE);
    if (configExists)
    {
        Serial.println("Motor config found.");

        File file = fs->open(MOTOR_CONFIG_FILE);
        bool success = load(&file);
        file.close();
        return success;
    }
    Serial.println("Motor config not found, loading default.");
    return false;
}

JsonDocument MotorConfig::json()
{
    JsonDocument jsonDocument;
    jsonDocument["reverseDirection"] = reverseDirection;
    jsonDocument["MICRO_STEPS"] = MICRO_STEPS;
    jsonDocument["STEPS_PER_ROT"] = STEPS_PER_ROT;
    jsonDocument["RMS_CURRENT"] = RMS_CURRENT;
    jsonDocument["hold_multiplier"] = hold_multiplier;
    jsonDocument["freewheel"] = freewheel;
    jsonDocument["AMAX_RPM_S"] = AMAX_RPM_S;
    jsonDocument["DMAX_RPM_S"] = DMAX_RPM_S;
    jsonDocument["VMAX_RPM"] = VMAX_RPM;
    jsonDocument["VSTOP"] = VSTOP;
    jsonDocument["VSTART"] = VSTART;
    jsonDocument["TOFF"] = TOFF;
    jsonDocument["SGT"] = SGT;
    jsonDocument["sfilt"] = sfilt;
    jsonDocument["sg_stop"] = sg_stop;
    jsonDocument["semin"] = semin;
    jsonDocument["semax"] = semax;
    jsonDocument["TCOOLTHRS_RPM"] = TCOOLTHRS_RPM;
    jsonDocument["TPWMTHRS_RPM"] = TPWMTHRS_RPM;
    jsonDocument["THIGH_RPM"] = THIGH_RPM;
    jsonDocument["VDCMIN_RPM"] = VDCMIN_RPM;
    jsonDocument["DC_TIME"] = DC_TIME;
    jsonDocument["DC_SG"] = DC_SG;
    jsonDocument["chm"] = chm;
    jsonDocument["en_pwm_mode"] = en_pwm_mode;
    jsonDocument["pwm_autoscale"] = pwm_autoscale;
    jsonDocument["pwm_autograd"] = pwm_autograd;
    jsonDocument["vhighfs"] = vhighfs;
    jsonDocument["vhighchm"] = vhighchm;
    jsonDocument["HSTRT"] = HSTRT;
    jsonDocument["HEND"] = HEND;
    jsonDocument["IHOLDDELAY"] = IHOLDDELAY;
    jsonDocument["TBL"] = TBL;
    jsonDocument["TPOWERDOWN"] = TPOWERDOWN;
    jsonDocument["TZEROWAIT"] = TZEROWAIT;
    jsonDocument["pid_P"] = pidP;
    jsonDocument["pid_I"] = pidI;
    jsonDocument["pid_D"] = pidD;
    jsonDocument["was_min"] = wasMin;
    jsonDocument["was_center"] = wasCenter;
    jsonDocument["was_max"] = wasMax;

    return jsonDocument;
}

bool MotorConfig::save(FS *fs)
{
    File file = fs->open(MOTOR_CONFIG_FILE, FILE_WRITE);
    if (!file)
    {
        Serial.printf("Couldn't write file: %s\n", MOTOR_CONFIG_FILE);
        return false;
    }
    serializeJson(json(), file);
    file.close();
    return true;
}

void MotorConfig::printToStream(Stream *stream)
{
    serializeJson(json(), *stream);
    stream->println();
}
void MotorConfig::printToStreamPretty(Stream *stream)
{
    serializeJsonPretty(json(), *stream);
    stream->println();
}

String MotorConfig::getDescription(String key)
{

    if (key == "hold_multiplier")
    {
        return "Multiplicator to get IHOLD (hold current) from IRUN (running current), set to 0 to allow freewheeling.";
    }
    else if (key == "reverseDirection")
    {
        return "Whether the motor should turn in the opposite direction.";
    }
    else if (key == "SGT")
    {
        return "StallGuard sensitivity threshold, a higher value decreases sensitivity and requres more torque to indicate a stall.";
    }
    else if (key == "RMS_CURRENT")
    {
        return "The RMS current rating of the motor, in mA. Keep this value at or below the motor's rating.";
    }
    else if (key == "en_pwm_mode")
    {
        return "Enable StealthChop when the speed is below TPWMTHRS_RPM.";
    }
    else if (key == "pwm_autoscale")
    {
        return "Enable automatic current control.";
    }
    else if (key == "pwm_autograd")
    {
        return "Enable automatic PWM gradient tuning.";
    }
    else if (key == "sfilt")
    {
        return "Enable filtering of StallGuard result. The result is updated every 4 fullsteps when enabled, otherwise every fullstep. The filtered result is more precise, but will be slower to observe a stall.";
    }
    else if (key == "sg_stop")
    {
        return "Enable stop by StallGuard, the motor will stop when encountering a stall.";
    }
    else if (key == "chm")
    {
        return "Chopper mode, disable to enable SpreadCycle, enable for constant off time with fast decay. Usually keep disabled.";
    }
    else if (key == "vhighfs")
    {
        return "Enable fullstepping when the speed is above THIGH_RPM.";
    }
    else if (key == "vhighchm")
    {
        return "Enable switching to chm=1 and fd=0 when speed is above THIGH_RPM to allow reaching higher speeds.";
    }
    else if (key == "freewheel")
    {
        return "Set freewheeling/braking mode when motor current is zero (I_HOLD=0).\n0: Normal operation\n1: Freewheeling\n2: Passive braking, LS drivers\n3: Passinve braking, HS drivers";
    }
    else if (key == "TBL")
    {
        return "Comparator blank time.\n0: 16t_clk\n1: 24t_clk\n2: 36t_clk\n3: 54t_clk\nTypically use 1 or 2.";
    }
    else if (key == "TOFF")
    {
        return "Enable the motor driver with TOFF>0. Controls duration of slow decay phase. If TOFF=1, use it only with TBL>=2.";
    }
    else if (key == "HSTRT")
    {
        return "Hysteresis chopper start value added to HEND";
    }
    else if (key == "HEND")
    {
        return "Hysteresis chopper low value.";
    }
    else if (key == "IHOLDDELAY")
    {
        return "Number of clock cycles for motor power down after stand still is detected and TPOWERDOWN has expired. 0 means instant powerdown, 1-15 gives longer delay. ";
    }
    else if (key == "TPOWERDOWN")
    {
        return "Sets the delay time from motor stand still to motor power down.";
    }
    else if (key == "TZEROWAIT")
    {
        return "Waiting time at zero velocity before switching direction.";
    }
    else if (key == "TOFF")
    {
        return "Enable the motor driver with TOFF>0. Controls duration of slow decay phase. If TOFF=1, use it only with TBL>=2.";
    }
    else if (key == "semin")
    {
        return "Increase the motor current if StallGuard result is below semin*32. Set above 0 to enable CoolStep.";
    }
    else if (key == "semax")
    {
        return "Decrease the motor current if StallGuard result is above (semin+semax)*32.";
    }
    else if (key == "DC_SG")
    {
        return "StallGuard sensitivity in DcStep mode. Higher value means higher sensitivity. Set slightly higher than DC_TIME/16.";
    }
    else if (key == "DC_TIME")
    {
        return "Pulsewidth for DcStep load measurement. Lower limit is TBL (clock cycles, not value) + n, n from 1 to 100";
    }
    else if (key == "VSTOP")
    {
        return "The stopping velocity of the motor. Usually keep higher than VSTART.";
    }
    else if (key == "VSTART")
    {
        return "The starting velocity of the motor.";
    }
    else if (key == "VMAX_RPM")
    {
        return "The max speed of the motor.";
    }
    else if (key == "TPWMTHRS_RPM")
    {
        return "The upper speed threshold for StealthChop mode. ";
    }
    else if (key == "TCOOLTHRS_RPM")
    {
        return "The lower speed threshold for enabling StallGuard and and priming CoolStep. CoolStep will enable when the speed is above THIGH_RPM.";
    }
    else if (key == "THIGH_RPM")
    {
        return "The lower speed threshold for changing chopper mode";
    }
    else if (key == "VDCMIN_RPM")
    {
        return "The lower speed threshold for DcStep. The motor will stop when falling below this when heavily loaded and sg_stop is enabled.";
    }
    else if (key == "AMAX_RPM_S")
    {
        return "The maximum acceleration rate, in RPM/s.";
    }
    else if (key == "DMAX_RPM_S")
    {
        return "The maximum deceleration rate, in RPM/s. This is typically set to 2-5x of AMAX_RPM_S.";
    }
    else if (key == "pid_P")
    {
        return "Proportional gain for PID controller.";
    }
    else if (key == "pid_I")
    {
        return "Integarl gain for PID controller.";
    }
    else if (key == "pid_D")
    {
        return "Derivative gain for PID controller.";
    }
    else if (key == "was_min")
    {
        return "Minimum reading value for the WAS (usually full left turn).";
    }
    else if (key == "was_center")
    {
        return "Center reading value for the WAS (wheels pointing straight).";
    }
    else if (key == "was_max")
    {
        return "Maximum reading value for the WAS (usually full right turn).";
    }

    return "Description missing.";
}