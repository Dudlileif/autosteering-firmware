#include "MotorConfig.h"
#include <ArduinoJson.h>

bool MotorConfig::load(Stream *stream)
{

    bool configReady = false;
    uint32_t waitStart = millis();
    Serial.println("Teensy waiting for update on serial");
    while (!configReady && millis() - waitStart < 2000)
    {
        if (stream->available())
        {
            Serial.println("Teensy found config on serial");
            configReady = true;
            break;
        }
    }
    if (!configReady)
    {
        return false;
    }
    StaticJsonDocument<1024> savedConfig;

    deserializeJson(savedConfig, *stream);

    Serial.println("Loading...");

    if (savedConfig.isNull())
    {
        Serial.println("Teensy config is empty, loading default.");
        return false;
    }
    if (savedConfig.containsKey("AMAX_RPM_S_2"))
    {
        AMAX_RPM_S_2 = savedConfig["AMAX_RPM_S_2"];
    }
    if (savedConfig.containsKey("VMAX_RPM"))
    {
        VMAX_RPM = savedConfig["VMAX_RPM"];
    }
    if (savedConfig.containsKey("VSTOP"))
    {
        VSTOP = savedConfig["VSTOP"];
    }
    if (savedConfig.containsKey("VSTART"))
    {
        VSTART = savedConfig["VSTART"];
    }
    if (savedConfig.containsKey("TOFF"))
    {
        TOFF = savedConfig["TOFF"];
    }
    if (savedConfig.containsKey("HSTRT"))
    {
        HSTRT = savedConfig["HSTRT"];
    }
    if (savedConfig.containsKey("HEND"))
    {
        HEND = savedConfig["HEND"];
    }
    if (savedConfig.containsKey("MICRO_STEPS"))
    {
        MICRO_STEPS = savedConfig["MICRO_STEPS"];
    }
    if (savedConfig.containsKey("STEPS_PER_ROT"))
    {
        STEPS_PER_ROT = savedConfig["STEPS_PER_ROT"];
    }
    if (savedConfig.containsKey("RMS_CURRENT"))
    {
        RMS_CURRENT = savedConfig["RMS_CURRENT"];
    }
    if (savedConfig.containsKey("hold_multiplier"))
    {
        hold_multiplier = savedConfig["hold_multiplier"];
    }
    if (savedConfig.containsKey("IHOLDDELAY"))
    {
        IHOLDDELAY = savedConfig["IHOLDDELAY"];
    }
    if (savedConfig.containsKey("freewheel"))
    {
        freewheel = savedConfig["freewheel"];
    }
    if (savedConfig.containsKey("TBL"))
    {
        TBL = savedConfig["TBL"];
    }
    if (savedConfig.containsKey("TPOWERDOWN"))
    {
        TPOWERDOWN = savedConfig["TPOWERDOWN"];
    }
    if (savedConfig.containsKey("TZEROWAIT"))
    {
        TZEROWAIT = savedConfig["TZEROWAIT"];
    }
    if (savedConfig.containsKey("en_pwm_mode"))
    {
        en_pwm_mode = savedConfig["en_pwm_mode"];
    }
    if (savedConfig.containsKey("pwm_autoscale"))
    {
        pwm_autoscale = savedConfig["pwm_autoscale"];
    }
    if (savedConfig.containsKey("pwm_autograd"))
    {
        pwm_autograd = savedConfig["pwm_autograd"];
    }
    if (savedConfig.containsKey("SGT"))
    {
        SGT = savedConfig["SGT"];
    }
    if (savedConfig.containsKey("sfilt"))
    {
        sfilt = savedConfig["sfilt"];
    }
    if (savedConfig.containsKey("sg_stop"))
    {
        sg_stop = savedConfig["sg_stop"];
    }
    if (savedConfig.containsKey("TPWMTHRS_RPM"))
    {
        TPWMTHRS_RPM = savedConfig["TPWMTHRS_RPM"];
    }
    if (savedConfig.containsKey("chm"))
    {
        chm = savedConfig["chm"];
    }
    if (savedConfig.containsKey("TCOOLTHRS_RPM"))
    {
        TCOOLTHRS_RPM = savedConfig["TCOOLTHRS_RPM"];
    }
    if (savedConfig.containsKey("semin"))
    {
        semin = savedConfig["semin"];
    }
    if (savedConfig.containsKey("semax"))
    {
        semax = savedConfig["semax"];
    }
    if (savedConfig.containsKey("THIGH_RPM"))
    {
        THIGH_RPM = savedConfig["THIGH_RPM"];
    }
    if (savedConfig.containsKey("vhighfs"))
    {
        vhighfs = savedConfig["vhighfs"];
    }
    if (savedConfig.containsKey("vhighchm"))
    {
        vhighchm = savedConfig["vhighchm"];
    }
    if (savedConfig.containsKey("VDCMIN_RPM"))
    {
        VDCMIN_RPM = savedConfig["VDCMIN_RPM"];
    }
    if (savedConfig.containsKey("DC_TIME"))
    {
        DC_TIME = savedConfig["DC_TIME"];
    }
    if (savedConfig.containsKey("DC_SG"))
    {
        DC_SG = savedConfig["DC_SG"];
    }
    if (savedConfig.containsKey("CAL_ROT"))
    {
        CAL_ROT = savedConfig["CAL_ROT"];
    }

    Serial.println("Teensy config loaded successfully.");

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

StaticJsonDocument<1024> MotorConfig::json()
{
    StaticJsonDocument<1024> jsonDocument;
    jsonDocument["MICRO_STEPS"] = MICRO_STEPS;
    jsonDocument["STEPS_PER_ROT"] = STEPS_PER_ROT;
    jsonDocument["RMS_CURRENT"] = RMS_CURRENT;
    jsonDocument["CAL_ROT"] = CAL_ROT;
    jsonDocument["hold_multiplier"] = hold_multiplier;
    jsonDocument["freewheel"] = freewheel;
    jsonDocument["AMAX_RPM_S_2"] = AMAX_RPM_S_2;
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
}
void MotorConfig::printToStreamPretty(Stream *stream)
{
    serializeJsonPretty(json(), *stream);
}

String MotorConfig::getDescription(String key)
{

    if (key == "hold_multiplier")
    {
        return "Multiplicator to get IHOLD (hold current) from IRUN (running current), set to 0 to allow freewheeling.";
    }
    else if (key == "CAL_ROT")
    {
        return "The number of rotations from center the motor should rotate during calibration. Keep low enough that the steering wheel doesn't reach the endstops.";
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
        return "The lower speed threshold for enabling StallGuard and and priming CoolStep. CoolStep will enable when the speed is above than THIGH_RPM.";
    }
    else if (key == "THIGH_RPM")
    {
        return "The lower speed threshold for changing chopper mode";
    }
    else if (key == "VDCMIN_RPM")
    {
        return "The lower speed threshold for DcStep. The motor will stop when falling below this when heavily loaded and sg_stop is enabled.";
    }
    else if (key == "AMAX_RPM_S_2")
    {
        return "The maximum acceleration rate, in RPM/s^2.";
    }

    return "Description missing.";
}