#ifndef MOTORCONFIG_H
#define MOTORCONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>

#define MOTOR_CONFIG_FILE "/motor_config"

#define DEFUALT_AMAX_RPM_S_2 100
#define DEFAULT_VMAX_RPM 200
#define DEFAULT_VSTOP 10
#define DEFAULT_VSTART 0
#define DEFAULT_TOFF 5
#define DEFAULT_HSTRT 4
#define DEFAULT_HEND 1
#define DEFAULT_MICRO_STEPS 256
#define DEFAULT_STEPS_PER_ROT 200
#define DEFAULT_RMS_CURRENT 1000
#define DEFAULT_HOLD_MUPLTIPLIER 0.0
#define DEFAULT_IHOLDDELAY 1
#define DEFAULT_FREEWHEEL 1
#define DEFAULT_TBL 1
#define DEFAULT_TPOWERDOWN 10
#define DEFAULT_TZEROWAIT 255
#define DEFAULT_EN_PWM_MODE true
#define DEFAULT_PWM_AUTOSCALE true
#define DEFAULT_PWM_AUTOGRAD false
#define DEFAULT_SFILT false
#define DEFAULT_SG_STOP 1
#define DEFAULT_SGT 16
#define DEFAULT_TPWMTHRS_RPM 40
#define DEFAULT_CHM 0
#define DEFAULT_TCOOLTHRS_RPM 10
#define DEFAULT_SEMIN 5
#define DEFAULT_SEMAX 2
#define DEFAULT_THIGH_RPM 100
#define DEFAULT_VHIGHFS 0
#define DEFAULT_VHIGHCHM 0
#define DEFAULT_VDCMIN_RPM 100
#define DEFAULT_DC_TIME 64
#define DEFAULT_DC_SG 5

#define DEFAULT_CAL_ROT 2.0

class MotorConfig
{
private:
public:
    // General
    // MICRO_STEPS between one full step
    uint16_t MICRO_STEPS;

    // General
    // Number of full steps for one rotation of the motor
    uint16_t STEPS_PER_ROT;

    // The motor current in mA
    uint16_t RMS_CURRENT;

    // The number of rotations from 0 the motor should
    // spin in both direactions  during calibration.
    float CAL_ROT;

    // 0.0...1.0, Mutliplier to get IHOLD from IRUN (IHOLD_IRUN).
    float hold_multiplier;

    // Stand still option when motor current setting is zero (IHOLD = 0).
    // %00: Normal operation
    // %01 : Freewheeling
    // %10 : Coil shorted using LS drivers
    // %11 : Coil shorted using HS drivers
    uint8_t freewheel;

    // Motion control AMAX
    // Maximum acceleration, RPM/s^2
    float AMAX_RPM_S_2;

    // Motion control VMAX
    // Maximum rpm (rpm=MICRO_STEPS/VMAX), (micro)steps/s
    float VMAX_RPM;

    // Motion control VSTOP
    // Stopping velocity, (micro)steps/s
    uint32_t VSTOP;

    // Motion control VSTART
    // Starting velocity, (micro)steps/s
    uint32_t VSTART;

    // General TOFF
    // Enable the driver with TOFF > 0
    uint8_t TOFF;

    // StallGuard SGT
    // Sensitivity range -64 -> 63 (high to low sensitivity)
    int8_t SGT;

    // Stallguard sfilt
    // Require 4 fullsteps for measurement
    bool sfilt;

    // Stallguard sg_stop
    // Enable Stallguard stop.
    bool sg_stop;

    // CoolStep semin
    // If sg_result < semin*32, current is increased
    uint8_t semin;

    // CoolStep semax
    // If sg_result > (semin+semax+1)*32, current is decreas
    uint8_t semax;

    // CoolStep TCOOLTHRS
    // Enable CoolStep when T < TCOOLTHRS
    float TCOOLTHRS_RPM;

    // SpreadCycle TWPMTHRS
    // Switch to SpreadCycle when T < TPWMTHRS
    float TPWMTHRS_RPM;

    // SpreadCycle chm
    // Which chopper mode to use, 0 is SpreadCycle,
    // 1 is constant off time with fast decay
    bool chm;

    // DcStep THIGH
    // Enable DcStep when T < THIGH, disables CoolStep and StallGuard
    float THIGH_RPM;

    // DcStep vhighfs
    // Enable fullstep at high velocities
    bool vhighfs;

    // DcStep vhighchm
    // If enabled, switches to chm(1) and fd(0) when exceeding VHIGH
    bool vhighchm;

    // DcStep VDCMIN
    // Enable DcStep when V > VDCMIN, (micro)steps/s
    float VDCMIN_RPM;

    // DcStep DC_TIME
    // Pulsewidth for DcStep load measurement
    uint16_t DC_TIME;

    // DcStep DC_SG
    // Stallguard sensitivity in DcStep mode
    uint8_t DC_SG;

    // 0...15 -> -3...12
    // chm=0
    // HEND hysteresis low value
    // (1/512 of this setting adds to current setting)
    // This is the hysteresis value which becomes
    // used for the hysteresis chopper.
    //
    // chm=1
    // OFFSET sine wave offset
    // This is the sine wave offset and 1 / 512 of the
    // value becomes added to the absolute value
    // of each sine wave entry.
    uint8_t HSTRT;

    // 0...15 -> -3...12
    // chm=0
    // HSTRT hysteresis start value added to HEND
    // Add 1, 2, ..., 8 to hysteresis low value HEND
    // (1 / 512 of this setting adds to current setting)
    // Attention : Effective HEND+ HSTRT ≤ 16.
    // Hint : Hysteresis decrement is done each 16 clocks
    //
    // chm=1
    // TFD [2..0] fast decay time setting
    // Fast decay time setting TFD with
    // N_CLK = 32 * TFD(%0000 : slow decay only)
    uint8_t HEND;

    // 0...15, IHOLDDELAY * 2^18 clock cycles after TZEROWAIT per current
    // decrement from run to hold, 0 means instant hold
    uint8_t IHOLDDELAY;

    // 0...3, Comparator blank time, 0: 16 t_clk, 1: 24 t_clk, 2: 36 t_clk, 3: 54 t_clk
    uint8_t TBL;

    // TPOWERDOWN sets the delay time after stand still (stst) of the
    // motor to motor current power down.Time range is about 0 to 4 seconds
    // Attention : A minimum setting of 2 is required to allow
    // automatic tuning of StealthChop PWM_OFS_AUTO.
    // 0...255 * 2 ^ 18*t_clk
    uint8_t TPOWERDOWN;

    // Defines the waiting time after ramping down to zero velocity before next movement or direction inversion can start.
    // Time range is about 0 to 2 seconds.
    // This setting avoids excess acceleration e.g.from VSTOP to - VSTART.
    // 0..65535 * 512*t_clk
    uint16_t TZEROWAIT;

    // StealthChop en_pwm_mode
    // Enable StealthChop when T > TPWMTHRS
    bool en_pwm_mode;

    // StealthChop pwm_autoscale
    // Enable automatic current control, PWM amplitude scaling
    bool pwm_autoscale;

    // StealthChop pwm_autograd
    // Enable automatic tuning of PWM_GRAD
    bool pwm_autograd;

    MotorConfig()
    {
        AMAX_RPM_S_2 = DEFUALT_AMAX_RPM_S_2;
        VMAX_RPM = DEFAULT_VMAX_RPM;
        VSTOP = DEFAULT_VSTOP;
        VSTART = DEFAULT_VSTART;
        TOFF = DEFAULT_TOFF;
        HSTRT = DEFAULT_HSTRT;
        HEND = DEFAULT_HEND;
        MICRO_STEPS = DEFAULT_MICRO_STEPS;
        STEPS_PER_ROT = DEFAULT_STEPS_PER_ROT;
        RMS_CURRENT = DEFAULT_RMS_CURRENT;
        hold_multiplier = DEFAULT_HOLD_MUPLTIPLIER;
        IHOLDDELAY = DEFAULT_IHOLDDELAY;
        freewheel = DEFAULT_FREEWHEEL;
        TBL = DEFAULT_TBL;
        TPOWERDOWN = DEFAULT_TPOWERDOWN;
        en_pwm_mode = DEFAULT_EN_PWM_MODE;
        pwm_autoscale = DEFAULT_PWM_AUTOSCALE;
        pwm_autograd = DEFAULT_PWM_AUTOGRAD;
        SGT = DEFAULT_SGT;
        sfilt = DEFAULT_SFILT;
        sg_stop = DEFAULT_SG_STOP;
        TPWMTHRS_RPM = DEFAULT_TPWMTHRS_RPM;
        chm = DEFAULT_CHM;
        TCOOLTHRS_RPM = DEFAULT_TCOOLTHRS_RPM;
        semin = DEFAULT_SEMIN;
        semax = DEFAULT_SEMAX;
        THIGH_RPM = DEFAULT_THIGH_RPM;
        vhighfs = DEFAULT_VHIGHFS;
        vhighchm = DEFAULT_VHIGHCHM;
        VDCMIN_RPM = DEFAULT_VDCMIN_RPM;
        DC_TIME = DEFAULT_DC_TIME;
        DC_SG = DEFAULT_DC_SG;
        CAL_ROT = DEFAULT_CAL_ROT;
    };
    ~MotorConfig(){};

    JsonDocument json();
    bool load(Stream *stream);
    bool loadFromFile(FS *fs);
    bool save(FS *fs);
    void printToStream(Stream *stream);
    void printToStreamPretty(Stream *stream);

    String getDescription(String key);
};

#endif