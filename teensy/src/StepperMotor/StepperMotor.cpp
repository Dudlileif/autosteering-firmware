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

#include "StepperMotor.h"

#include "Config/Config.h"
#include "Comms/Comms.h"
#include "Sensors/Sensors.h"

MotorConfig motorConfig;

PidController pidController;

bool motorEnabled = false;
bool motorCalibration = false;
bool motorStalled = false;

uint16_t wasTarget = 0;

StepperRampMode stepperRampMode = positive;
uint32_t stepperVMax = 0;

float stepperRPMActual = 0;
int32_t stepperPositionActual = 0;
uint16_t stepperStallguardResult = 0;
uint8_t stepperCurrentScale = 0;

bool calibrationTarget = 0;

elapsedMicros stepperElapsedTime;
elapsedMicros stepperLastCommandElapsedTime;

const uint32_t f_clk = 12000000;

const float t_vel = 1.398101; // pow(2, 24) / f_clk
const float t_acc = 0.015271; // pow(2, 41) / pow(f_clk, 2)

// On TMC5160 BOB                          CSN   (r_sense, ohm)  SDI           SDO           SCK    (link_address)
TMC5160Stepper stepper = TMC5160Stepper(PIN_SPI_SS1, 0.075, PIN_SPI_MOSI1, PIN_SPI_MISO1, PIN_SPI_SCK1, -1);

elapsedMicros stallElapsedTime;

void stepperInit()
{
    pinMode(MOTOR_ENABLE_PIN, OUTPUT);

    stepper.begin(); // Start SPI connection with driver
    stepper.setSPISpeed(4000000);

    updateStepperDriverConfig();

    Serial.println("Stepper initialized.");
    restartStepper();
}

void initPidController()
{
    pidController.clear();
    pidController.p = motorConfig.pidP;
    pidController.i = motorConfig.pidI;
    pidController.d = motorConfig.pidD;
}

void updateStepperDriverConfig()
{
    stepper.toff(motorConfig.TOFF);                       // Enable the driver with toff > 0
    stepper.microsteps(motorConfig.MICRO_STEPS);          // Set MICRO_STEPS
    stepper.hold_multiplier(motorConfig.hold_multiplier); // 0..1, multiplier to get IHOLD from IRUN.
    stepper.rms_current(motorConfig.RMS_CURRENT);         // Set the motor current (mA)
    stepper.iholddelay(motorConfig.IHOLDDELAY);           // 0..15, IHOLDDELAY * 2^18 clock cycles after TZEROWAIT per current decrement from run to hold, 0 means instant hold
    stepper.freewheel(motorConfig.freewheel);             // Stand still option when motor current is zero, 0: normal op., 1: freewheeling, 2: coil shorted with LS drivers,
    // 3: coil shorted with HS driver
    stepper.tbl(motorConfig.TBL);     // 0..3, Comparator blank time, 0: 16 t_clk, 1: 24 t_clk, 2: 36 t_clk, 3: 54 t_clk
    stepper.hstrt(motorConfig.HSTRT); // 0...15 -> -3...12, Hysteresis start value added to HEND.
    stepper.hend(motorConfig.HEND);   // 0...15 -> -3...12, Hysteresis low value.
    // Sets the delay time after stand still until the motor current powers
    // down.
    // 0..255 * 2^18*t_clk
    stepper.TPOWERDOWN(motorConfig.TPOWERDOWN);

    // Ramp acceleration and speed config
    stepper.VSTART(motorConfig.VSTART);
    stepper.VSTOP(motorConfig.VSTOP);
    stepper.AMAX(accelerationFromRPMS(motorConfig.AMAX_RPM_S));
    stepper.DMAX(accelerationFromRPMS(motorConfig.DMAX_RPM_S));
    stepper.v1(velocityFromRPM(motorConfig.VMAX_RPM / 2));
    stepper.a1(accelerationFromRPMS(motorConfig.AMAX_RPM_S / 2));
    stepper.d1(accelerationFromRPMS(motorConfig.AMAX_RPM_S / 2));

    // Sets the wait time at zero velocity after a ramp before continuing
    // to the other side of zero.
    // 0..65535 * 512*t_clk
    stepper.TZEROWAIT(motorConfig.TZEROWAIT);

    // StealthChop
    stepper.en_pwm_mode(motorConfig.en_pwm_mode);     // Enable StealthChop when T > TPWMTHRS
    stepper.pwm_autoscale(motorConfig.pwm_autoscale); // Enable automatic current control, PWM amplitude scaling
    stepper.pwm_autograd(motorConfig.pwm_autograd);   // Enable automatic PWM gradient adaptation

    // Stallguard and CoolStep
    stepper.sfilt(motorConfig.sfilt); // Require 4 fullsteps for measurement
    stepper.sgt(motorConfig.SGT);     // Range -64 -> 63 (high to low sensitivity)
    stepper.sg_stop(motorConfig.sg_stop);
    stepper.TCOOLTHRS(tFromRPM(motorConfig.TCOOLTHRS_RPM)); // Enable Stallguard and CoolStep when RPM > TCOOLTHRS_RPM
    stepper.semin(motorConfig.semin);                       // If sg_result < semin*32, current is increased
    stepper.semax(motorConfig.semax);                       // If sg_result > (semin+semax+1)*32, current is decreased

    // SpreadCycle
    stepper.TPWMTHRS(tFromRPM(motorConfig.TPWMTHRS_RPM)); // Switch to SpreadCycle when T < TPWMTHRS
    stepper.chm(motorConfig.chm);

    // DC step
    stepper.THIGH(tFromRPM(motorConfig.THIGH_RPM)); // Enable DcStep when T < 100, disables CoolStep and StallGuard
    stepper.vhighfs(motorConfig.vhighfs);           // Enable fullstep at high velocities
    stepper.vhighchm(motorConfig.vhighchm);         // Switches to chm(1) and fd(0) when exceeding VHIGH
    stepper.VDCMIN(tFromRPM(motorConfig.VDCMIN_RPM));
    stepper.dc_time(motorConfig.DC_TIME);
    stepper.dc_sg(motorConfig.DC_SG);

    initPidController();

    Serial.println("Updated stepper driver config");
}

int32_t positionFromRotations(float rotations)
{
    return round(rotations * (motorConfig.MICRO_STEPS == 0 ? 1 : motorConfig.MICRO_STEPS) * motorConfig.STEPS_PER_ROT);
}

float rotationsFromPosition(int32_t position)
{
    return float(position) / (motorConfig.MICRO_STEPS == 0 ? 1 : motorConfig.MICRO_STEPS) / motorConfig.STEPS_PER_ROT;
}

uint32_t velocityFromRPM(float rpm)
{
    return round(rpm / 60 * t_vel * (motorConfig.MICRO_STEPS == 0 ? 1 : motorConfig.MICRO_STEPS) * motorConfig.STEPS_PER_ROT);
}

float rpmFromVelocity(int32_t velocity)
{
    return velocity * 60 / t_vel / (motorConfig.MICRO_STEPS == 0 ? 1 : motorConfig.MICRO_STEPS) / motorConfig.STEPS_PER_ROT;
}

uint32_t tFromVelocity(uint32_t velocity)
{
    return constrain(pow(2, 24) / (velocity * 256 / (motorConfig.MICRO_STEPS == 0 ? 1 : motorConfig.MICRO_STEPS)), 0, pow(2, 20) - 1);
}

uint32_t tFromRPM(float rpm)
{
    return tFromVelocity(velocityFromRPM(rpm));
}

uint32_t velocityFromT(uint32_t t)
{
    return pow(2, 24) / (t * 256 / (motorConfig.MICRO_STEPS == 0 ? 1 : motorConfig.MICRO_STEPS));
}

float rpmFromT(uint32_t t)
{
    return rpmFromVelocity(velocityFromT(t));
}

uint32_t accelerationFromRPMS(float rpms)
{
    return constrain(velocityFromRPM(rpms) * t_acc, 0, pow(2, 16) - 1);
}

float rpmsFromAcceleration(uint32_t acceleration)
{
    return rpmFromVelocity(acceleration) / t_acc;
}

void restartStepper()
{
    pidController.clear();
    stepper.toff(0);
    stepper.sg_stop(0);
    delay(10);
    stepper.sg_stop(motorConfig.sg_stop);
    stepper.toff(motorConfig.TOFF);

    motorStalled = false;

    Serial.println("Stepper restarted.");
}

void handleMotorControls(JsonDocument &document)
{
    JsonObject data = document.as<JsonObject>();

    JsonVariant wasTargetPos = data["was_target"];
    JsonVariant enableMotor = data["enable_motor"];
    JsonVariant calibrate = data["motor_en_cal"];

    if (!wasTargetPos.isNull())
    {
        wasTarget = constrain(wasTargetPos, motorConfig.wasMin, motorConfig.wasMax);
        if (!enableMotor.isNull())
        {
            if (motorEnabled != enableMotor && stallElapsedTime > 5e6)
            {
                restartStepper();
            }
            motorEnabled = enableMotor;
        }
    }
    else if (!calibrate.isNull())
    {
        if (!motorCalibration)
        {
            restartStepper();
            motorEnabled = true;
        }
        motorCalibration = calibrate;
    }
    else
    {
        if (motorEnabled)
        {
            Serial.println("Motor stopped.");
        }
        motorCalibration = false;
        motorEnabled = false;
        // char message[] = "{\"motor_enabled\":false}";
        // sendToProgram(message, sizeof(message));
    }
    stepperLastCommandElapsedTime = 0;
}

float normalizeWasReading(uint16_t reading)
{
    float normalizedWasReading = 0.5;

    if (reading < motorConfig.wasCenter)
    {
        normalizedWasReading = 0.5 * (reading - motorConfig.wasMin) / (motorConfig.wasCenter - motorConfig.wasMin);
    }
    else if (reading > motorConfig.wasCenter)
    {
        normalizedWasReading = 0.5 + 0.5 * (reading - motorConfig.wasCenter) / (motorConfig.wasMax - motorConfig.wasCenter);
    }

    return constrain(normalizedWasReading, 0.0, 1.0);
}

float asymmetricCoefficient(uint16_t reading)
{
    if (motorConfig.asymmetricVelocity)
    {
        float leftSide = float(motorConfig.wasCenter - motorConfig.wasMin);
        float rightSide = float(motorConfig.wasMax - motorConfig.wasCenter);
        if (reading < motorConfig.wasCenter && leftSide < rightSide)
        {
            return leftSide / rightSide;
        }
        else if (reading > motorConfig.wasCenter && rightSide < leftSide)
        {
            return rightSide / leftSide;
        }
    }
    return 1;
}

void updateStepper()
{
    digitalWrite(MOTOR_ENABLE_PIN, motorEnabled);
    if (stepperLastCommandElapsedTime > STEPPER_COMMAND_UPDATE_US)
    {
        if (motorEnabled)
        {
            stepperVMax = 0;
            motorEnabled = false;
            Serial.println("Motor stopped, too long since last command.");
            char message[] = "{\"motor_no_command\": true}";
            sendToProgram(message, sizeof(message));
        }
        motorCalibration = false;
    }
    if (stepperElapsedTime > STEPPER_PERIOD_US)
    {
        stepperElapsedTime = 0;
        stepperStallguardResult = stepper.sg_result();
        stepperCurrentScale = stepper.cs_actual();
        stepperRPMActual = rpmFromVelocity(stepper.VACTUAL());
        stepperPositionActual = stepper.XACTUAL();

        if (motorEnabled)
        {
            float normalizedReading = normalizeWasReading(wasReading);

            if (abs(stepperRPMActual) > motorConfig.TCOOLTHRS_RPM && stepper.stallguard() && !motorStalled)
            {
                Serial.println("Motor stalled!");
                motorEnabled = false;
                // char message[26] = "{\"motor_stalled\": true}";
                // sendToProgram(message, sizeof(message));
                stallElapsedTime = 0;
                motorStalled = true;
            }
            else
            {
                float velocityGain = 0;
                if (motorCalibration)
                {
                    float difference = sqrt(
                        pow(normalizedReading - calibrationTarget, 2));
                    if (difference < 0.05)
                    {
                        if (calibrationTarget < 0.5)
                        {
                            calibrationTarget = 1;
                        }
                        else if (calibrationTarget > 0.5)
                        {
                            calibrationTarget = 0;
                        }
                    }
                    velocityGain = constrain(pidController.next(calibrationTarget - normalizedReading), -1, 1) * asymmetricCoefficient(wasReading);
                }
                else
                {

                    float normalizedTarget = normalizeWasReading(wasTarget);
                    float pidValue = pidController.next(normalizedTarget - normalizedReading);
                    velocityGain = constrain(pidValue, -1, 1) * asymmetricCoefficient(wasReading);
                }
                stepperVMax = abs(velocityGain) * velocityFromRPM(motorConfig.VMAX_RPM);
                StepperRampMode mode = velocityGain >= 0 ? positive : negative;
                if (motorConfig.reverseDirection)
                {
                    mode = mode == positive ? negative : positive;
                }
                stepper.AMAX(accelerationFromRPMS(stepperVMax < abs(stepper.VACTUAL()) ? motorConfig.DMAX_RPM_S : motorConfig.AMAX_RPM_S));
                stepper.VMAX(motorEnabled ? stepperVMax : 0);
                stepper.RAMPMODE(mode);
            }
        }
        else if (motorCalibration && stallElapsedTime > 5e6)
        {
            restartStepper();
            motorEnabled = true;
        }
        else
        {
            stepper.VMAX(0);
        }
    }
}

// TODO: Add stall when steering is not responding (manual takeover)