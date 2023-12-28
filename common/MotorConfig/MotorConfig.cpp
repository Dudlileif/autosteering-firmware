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
    if (savedConfig.containsKey("AMAX"))
    {
        AMAX = savedConfig["AMAX"];
    }
    if (savedConfig.containsKey("RPMMAX"))
    {
        RPMMAX = savedConfig["RPMMAX"];
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
    if (savedConfig.containsKey("IHOLD_IRUN"))
    {
        IHOLD_IRUN = savedConfig["IHOLD_IRUN"];
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
    if (savedConfig.containsKey("pwm_auto_scale"))
    {
        pwm_autoscale = savedConfig["pwm_auto_scale"];
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
    if (savedConfig.containsKey("TPWMTHRS"))
    {
        TPWMTHRS = savedConfig["TPWMTHRS"];
    }
    if (savedConfig.containsKey("chm"))
    {
        chm = savedConfig["chm"];
    }
    if (savedConfig.containsKey("TCOOLTHRS"))
    {
        TCOOLTHRS = savedConfig["TCOOLTHRS"];
    }
    if (savedConfig.containsKey("semin"))
    {
        semin = savedConfig["semin"];
    }
    if (savedConfig.containsKey("semax"))
    {
        semax = savedConfig["semax"];
    }
    if (savedConfig.containsKey("THIGH"))
    {
        THIGH = savedConfig["THIGH"];
    }
    if (savedConfig.containsKey("vhighfs"))
    {
        vhighfs = savedConfig["vhighfs"];
    }
    if (savedConfig.containsKey("vhighchm"))
    {
        vhighchm = savedConfig["vhighchm"];
    }
    if (savedConfig.containsKey("VDCMIN"))
    {
        VDCMIN = savedConfig["VDCMIN"];
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
        Serial.println("Teensy config found.");

        File file = fs->open(MOTOR_CONFIG_FILE);
        bool success = load(&file);
        file.close();
        return success;
    }
    Serial.println("Teensy config not found, loading default.");
    return false;
}

StaticJsonDocument<1024> MotorConfig::json()
{
    StaticJsonDocument<1024> jsonDocument;

    jsonDocument["AMAX"] = AMAX;
    jsonDocument["RPMMAX"] = RPMMAX;
    jsonDocument["VSTOP"] = VSTOP;
    jsonDocument["VSTART"] = VSTART;
    jsonDocument["TOFF"] = TOFF;
    jsonDocument["HSTRT"] = HSTRT;
    jsonDocument["HEND"] = HEND;
    jsonDocument["MICRO_STEPS"] = MICRO_STEPS;
    jsonDocument["STEPS_PER_ROT"] = STEPS_PER_ROT;
    jsonDocument["RMS_CURRENT"] = RMS_CURRENT;
    jsonDocument["IHOLD_IRUN"] = IHOLD_IRUN;
    jsonDocument["hold_multiplier"] = hold_multiplier;
    jsonDocument["IHOLDDELAY"] = IHOLDDELAY;
    jsonDocument["TBL"] = TBL;
    jsonDocument["TPOWERDOWN"] = TPOWERDOWN;
    jsonDocument["TZEROWAIT"] = TZEROWAIT;
    jsonDocument["en_pwm_mode"] = en_pwm_mode;
    jsonDocument["pwm_auto_scale"] = pwm_autoscale;
    jsonDocument["SGT"] = SGT;
    jsonDocument["sfilt"] = sfilt;
    jsonDocument["sg_stop"] = sg_stop;
    jsonDocument["TPWMTHRS"] = TPWMTHRS;
    jsonDocument["chm"] = chm;
    jsonDocument["TCOOLTHRS"] = TCOOLTHRS;
    jsonDocument["semin"] = semin;
    jsonDocument["semax"] = semax;
    jsonDocument["THIGH"] = THIGH;
    jsonDocument["vhighfs"] = vhighfs;
    jsonDocument["vhighchm"] = vhighfs;
    jsonDocument["VDCMIN"] = VDCMIN;
    jsonDocument["DC_TIME"] = DC_TIME;
    jsonDocument["DC_SG"] = DC_SG;
    jsonDocument["CAL_ROT"] = CAL_ROT;

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
