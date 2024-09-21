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

#ifndef MOTORCONFIG_H
#define MOTORCONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>

#define MOTOR_CONFIG_FILE "/motor_config"

#define DEFAULT_REVERSE_DIRECTION false
#define DEFUALT_AMAX_RPM_S 100
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
#define DEFAULT_SG_STOP true
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

#define DEFAULT_PID_P 3.0
#define DEFAULT_PID_I 0.0
#define DEFAULT_PID_D 0.0

#define WAS_MIN 250
#define WAS_CENTER 2000
#define WAS_MAX 3750

/**
 * A configuration class for coniguring a TMC5160 driver and the connected
 * stepper motor.
 */
class MotorConfig
{
private:
public:
    /**
     * Number of full steps for one rotation of the motor, usually 200.
     */
    uint16_t STEPS_PER_ROT;

    /**
     * Whether the motor direction should be reversed.
     */
    bool reverseDirection;

    /**
     * The motor current in mA.
     */
    uint16_t RMS_CURRENT;

    /**
     * Microsteps to divide a full step into.
     *
     * CHOPCONF MRES in driver docs.
     */
    uint16_t MICRO_STEPS;

    /**
     * 0.0...1.0, Mutliplier to get IHOLD from IRUN (IHOLD_IRUN).
     */
    float hold_multiplier;

    /**
     * Stand still option when motor current setting is zero (IHOLD = 0).
     * %00: Normal operation
     * %01 : Freewheeling
     * %10 : Coil shorted using LS drivers
     * %11 : Coil shorted using HS drivers
     *
     * PWMCONF freewheel in driver docs.
     */
    uint8_t freewheel;

    /**
     * Maximum acceleration, RPM/s.
     *
     * Ramp Generator Motion Control Register Set AMAX in driver docs.
     */
    float AMAX_RPM_S;

    /**
     * Maximum RPM (RPM=MICRO_STEPS/VMAX), (micro)steps/s.
     *
     * Ramp Generator Motion Control Register Set VMAX in driver docs.
     */
    float VMAX_RPM;

    /**
     * Stopping velocity, (micro)steps/s.
     *
     * Ramp Generator Motion Control Register Set VSTOP in driver docs.
     */
    uint32_t VSTOP;

    /**
     * Starting velocity, (micro)steps/s.
     *
     * Ramp Generator Motion Control Register Set VSTART in driver docs.
     */
    uint32_t VSTART;

    /**
     * Enable the driver with TOFF > 0.
     *
     * CHOPCONF TOFF in driver docs.
     */
    uint8_t TOFF;

    /**
     * Sensitivity range -64 -> 63 (high to low sensitivity).
     *
     * COOLCONF SGT in driver docs.
     */
    int8_t SGT;

    /**
     * Require 4 fullsteps for measurement of StallGuard measurement.
     *
     * COOLCONF sfilt in driver docs.
     */
    bool sfilt;

    /**
     * Enable StallGuard stop when stall is detected.
     *
     * SW_MODE sg_stop in driver docs.
     */
    bool sg_stop;

    /**
     * If sg_result < semin*32, current is increased.
     *
     * COOLCONF semin in driver docs.
     */
    uint8_t semin;

    /** If sg_result > (semin+semax+1)*32, current is decreased.
     *
     * COOLCONF semax in driver docs.
     */
    uint8_t semax;

    /** Enable CoolStep when T < TCOOLTHRS, i.e. V_RPM > TCOOLTHRS_RPM
     *
     * CoolStep TCOOLTHRS in driver docs.
     */
    float TCOOLTHRS_RPM;

    /**
     * Upper velocity threshold for StealthChop.
     *
     * Velocity Dependent Driver Feature Control Register Set TWPMTHRS in driver docs.
     */
    float TPWMTHRS_RPM;

    /**
     * Which chopper mode to use, 0 is SpreadCycle,
     * 1 is constant off time with fast decay.
     *
     * CHOPCPNF chm in driver docs.
     */
    bool chm;

    /**
     * Enable DcStep when T < THIGH, i.e. V_RPM > THIGH_RPM, also disables CoolStep
     * and StallGuard.
     *
     * Velocity Dependent Driver Feature Control Register Set THIGH in driver docs.
     */
    float THIGH_RPM;

    /**
     * Enable fullstep when V_RPM > THIGH_RPM.
     *
     * CHOPCONF vhighfs in driver docs.
     */
    bool vhighfs;

    /**
     * If enabled, switches to chm(1) and fd(0) when V_RPM > THIGH_RPM.
     *
     * CHOPCONF vhighchm in driver docs.
     */
    bool vhighchm;

    /**
     * Enable DcStep when V_RPM > VDCMIN_RPM, (micro)steps/s.
     *
     * Ramp Generator Driver Feature Control Register Set VDCMIN in driver docs.
     */
    float VDCMIN_RPM;

    /**
     * Pulsewidth for DcStep load measurement.
     *
     * DRIVER REGISTER SET DCCTRL DC_TIME in driver docs.
     */
    uint16_t DC_TIME;

    /**
     * Stallguard sensitivity in DcStep mode.
     *
     * DRIVER REGISTER SET DCCTRL DC_SG in driver docs.
     */
    uint8_t DC_SG;

    /**
     * 0...15 -> -3...12
     * chm=0
     * HEND hysteresis low value
     * (1/512 of this setting adds to current setting)
     * This is the hysteresis value which becomes
     * used for the hysteresis chopper.
     *
     * chm=1
     * OFFSET sine wave offset
     * This is the sine wave offset and 1 / 512 of the
     * value becomes added to the absolute value
     * of each sine wave entry.
     *
     * CHOPCONF HSTRT in driver docs.
     */
    uint8_t HSTRT;

    /**
     * 0...15 -> -3...12
     * chm=0
     * HSTRT hysteresis start value added to HEND
     * Add 1, 2, ..., 8 to hysteresis low value HEND
     * (1 / 512 of this setting adds to current setting)
     * Attention : Effective HEND+ HSTRT ≤ 16.
     * Hint : Hysteresis decrement is done each 16 clocks
     *
     * chm=1
     * TFD [2..0] fast decay time setting
     * N_CLK = 32 * TFD(%0000 : slow decay only)
     * Fast decay time setting TFD with
     *
     * CHOPCONF HEND in driver docs.
     */
    uint8_t HEND;

    /**
     * 0...15, IHOLDDELAY * 2^18 clock cycles after TZEROWAIT per current
     * decrement from run to hold, 0 means instant hold
     *
     * Velocity Dependent Driver Feature Control Register Set IHOLD_IRUN
     * IHOLDDELAY in driver docs.
     */
    uint8_t IHOLDDELAY;

    /**
     * 0...3, Comparator blank time, 0: 16 t_clk, 1: 24 t_clk, 2: 36 t_clk, 3: 54 t_clk
     *
     * CHOPCONF TBL in driver docs.
     */
    uint8_t TBL;

    /**
     * TPOWERDOWN sets the delay time after stand still (stst) of the
     * motor to motor current power down.Time range is about 0 to 4 seconds
     * Attention : A minimum setting of 2 is required to allow
     * automatic tuning of StealthChop PWM_OFS_AUTO.
     * 0...255 * 2 ^ 18*t_clk
     *
     * Velocity Dependent Driver Feature Control Register Set TPOWERDOWN in driver docs.
     */
    uint8_t TPOWERDOWN;

    /**
     * Defines the waiting time after ramping down to zero velocity before next movement or direction
     * inversion can start.
     * Time range is about 0 to 2 seconds.
     * This setting avoids excess acceleration e.g.from VSTOP to -VSTART.
     * 0..65535 * 512*t_clk
     *
     * Ramp Generator Motion Control Register Set TZEROWAIT in driver docs.
     */
    uint16_t TZEROWAIT;

    /**
     * Enable StealthChop when T > TPWMTHRS, i.e. V<TPWMTHRS_RPM
     *
     * GCONF en_pwm_mode in driver docs.
     */
    bool en_pwm_mode;

    /**
     * StealthChop pwm_autoscale
     * Enable automatic current control, PWM amplitude scaling
     *
     * PWMCONF pwm_autoscale in driver docs.
     */
    bool pwm_autoscale;

    /**
     * StealthChop pwm_autograd
     * Enable automatic tuning of PWM_GRAD
     *
     * PWMCONF pwm_autograd in driver docs.
     */
    bool pwm_autograd;

    /**
     * Proportional PID controller gain.
     */
    float pidP;

    /**
     * Integral PID controller gain.
     */
    float pidI;

    /**
     * Derivative PID controller gain.
     */
    float pidD;

    /**
     * Minium reading value for the WAS (usually full left turn).
     */
    uint16_t wasMin;

    /**
     * Minium reading value for the WAS (wheels pointing straight).
     */
    uint16_t wasCenter;

    /**
     * Maximum reading value for the WAS (usually full right turn).
     */
    uint16_t wasMax;

    /**
     * Default constructor, all parameters are set to the default values.
     */
    MotorConfig()
    {
        reverseDirection = DEFAULT_REVERSE_DIRECTION;
        AMAX_RPM_S = DEFUALT_AMAX_RPM_S;
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
        pidP = DEFAULT_PID_P;
        pidI = DEFAULT_PID_I;
        pidD = DEFAULT_PID_D;
        wasMin = WAS_MIN;
        wasCenter = WAS_CENTER;
        wasMax = WAS_MAX;
    };
    /**
     * Default deconstructor.
     */
    ~MotorConfig() {};

    /**
     * Create a JSON representation of this.
     */
    JsonDocument json();

    /**
     * Load the motor configuration from a stream.
     *
     * @param stream The stream to read from.
     */
    bool load(Stream *stream);

    /**
     * Load the motor configuration from the file at path MOTOR_CONFIG_FILE.
     *
     * @param fs The file system implementation to read with.
     */
    bool loadFromFile(FS *fs);

    /**
     * Save this to the file at path MOTOR_CONFIG_FILE.
     *
     * @param fs the file system implementation to save with.
     */
    bool save(FS *fs);

    /**
     * Print the JSON representation of this to the stream.
     *
     * @param stream Where to print.
     */
    void printToStream(Stream *stream);

    /**
     * Pretty print the JSON representation of this to the stream.
     *
     * @param stream Where to print.
     */
    void printToStreamPretty(Stream *stream);

    /**
     * Returns a description of the parameter with the given JSON
     * key.
     *
     * @param key The JSON key to get the description from.
     */
    String getDescription(String key);
};

#endif