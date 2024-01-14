#include "StepperMotor.h"

#include "Config/Config.h"
#include "Comms/Comms.h"

MotorConfig motorConfig;

bool motorEnabled = false;
bool motorCalibration = false;

float stepperTargetRPM = 0;
int32_t stepperTargetPosition = 0;
StepperRampMode stepperRampMode = positive;
uint32_t stepperVMax = 0;

float stepperRPMActual = 0;
int32_t stepperPositionActual = 0;
uint16_t stepperStallguardResult = 0;
uint8_t stepperCurrentScale = 0;

unsigned long stepperPrevUpdateTime = 0;
unsigned long stepperPrevCommandTime = 0;

const uint32_t f_clk = 12000000;

const float t_vel = 1.398101; // pow(2, 24) / f_clk
const float t_acc = 0.015271; // pow(2, 41) / pow(f_clk, 2)

// On TMC5160 BOB                          CSN   (r_sense, ohm)  SDI           SDO           SCK    (link_address)
TMC5160Stepper stepper = TMC5160Stepper(PIN_SPI_SS1, 0.075, PIN_SPI_MOSI1, PIN_SPI_MISO1, PIN_SPI_SCK1, -1);

uint32_t motorStallTime = 0;
int32_t motorStartPosition = 0;
float calRotStartFraction = 0; // -1...1, position of starting point in the available range

void stepperInit()
{
    pinMode(MOTOR_ENABLE_PIN, OUTPUT);

    stepper.begin(); // Start SPI connection with driver
    stepper.setSPISpeed(4000000);
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
    stepper.AMAX(accelerationFromRPMS2(motorConfig.AMAX_RPM_S_2));
    stepper.DMAX(accelerationFromRPMS2(motorConfig.AMAX_RPM_S_2));
    stepper.v1(velocityFromRPM(motorConfig.VMAX_RPM / 2));
    stepper.a1(accelerationFromRPMS2(motorConfig.AMAX_RPM_S_2));
    stepper.d1(accelerationFromRPMS2(motorConfig.AMAX_RPM_S_2));

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

    Serial.println("Stepper initialized.");
    restartStepper();
}

int32_t positionFromRotations(float rotations)
{
    return round(rotations * motorConfig.MICRO_STEPS * motorConfig.STEPS_PER_ROT);
}

float rotationsFromPosition(int32_t position)
{
    return float(position) / motorConfig.MICRO_STEPS / motorConfig.STEPS_PER_ROT;
}

uint32_t velocityFromRPM(float rpm)
{
    return round(rpm / 60 * t_vel * motorConfig.MICRO_STEPS * motorConfig.STEPS_PER_ROT);
}

float rpmFromVelocity(int32_t velocity)
{
    return velocity * 60 / t_vel / motorConfig.MICRO_STEPS / motorConfig.STEPS_PER_ROT;
}

uint32_t tFromVelocity(uint32_t velocity)
{
    return constrain(pow(2, 24) / (velocity * 256 / motorConfig.MICRO_STEPS), 0, pow(2, 20) - 1);
}

uint32_t tFromRPM(float rpm)
{
    return tFromVelocity(velocityFromRPM(rpm));
}

uint32_t velocityFromT(uint32_t t)
{
    return pow(2, 24) / (t * 256 / motorConfig.MICRO_STEPS);
}

float rpmFromT(uint32_t t)
{
    return rpmFromVelocity(velocityFromT(t));
}

uint32_t accelerationFromRPMS2(float rpms2)
{
    return constrain(velocityFromRPM(rpms2) * t_acc, 0, pow(2, 16) - 1);
}

float rpms2FromAcceleration(uint32_t acceleration)
{
    return rpmFromVelocity(acceleration) / t_acc;
}

void restartStepper()
{
    stepper.toff(0);
    stepper.sg_stop(0);
    stepper.sg_stop(motorConfig.sg_stop);
    stepper.toff(motorConfig.TOFF);
    Serial.println("Stepper restarted.");
}

void setCalRotFraction()
{
    int32_t minPos = motorStartPosition - (1 + calRotStartFraction) * positionFromRotations(motorConfig.CAL_ROT);

    calRotStartFraction = constrain(2 * (stepperPositionActual - minPos) / float(2 * positionFromRotations(motorConfig.CAL_ROT)) - 1, -1, 1);
}

void handleMotorControls(DynamicJsonDocument &document)
{
    JsonObject data = document.as<JsonObject>();

    JsonVariant motorRPM = data["motor_rpm"];
    JsonVariant enableMotor = data["enable_motor"];
    JsonVariant calibrate = data["motor_en_cal"];

    if (!motorRPM.isNull())
    {
        stepperTargetRPM = motorRPM;

        if (!enableMotor.isNull())
        {
            if (motorEnabled != enableMotor && micros() - motorStallTime > 5e6)
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
        motorCalibration = false;
        motorEnabled = false;
        Serial.println("Motor stopped.");
        char message[] = "{\"motor_enabled\": false}";
        sendToProgram(message, sizeof(message));
    }
    stepperPrevCommandTime = micros();
}

void updateStepper()
{
    digitalWrite(MOTOR_ENABLE_PIN, !motorEnabled);

    unsigned long now = micros();
    if (now - stepperPrevCommandTime > STEPPER_COMMAND_UPDATE_US)
    {
        if (motorEnabled)
        {
            stepperVMax = 0;
            stepperTargetRPM = 0;
            motorEnabled = false;
            Serial.println("Motor stopped, too long since last command.");
            setCalRotFraction();
            char message[] = "{\"motor_no_command\": true}";
            sendToProgram(message, sizeof(message));
        }
        motorCalibration = false;
    }
    if (now - stepperPrevUpdateTime > STEPPER_PERIOD_US)
    {
        stepperPrevUpdateTime = now;

        stepperStallguardResult = stepper.sg_result();
        stepperCurrentScale = stepper.cs_actual();

        stepperRPMActual = rpmFromVelocity(stepper.VACTUAL());
        stepperPositionActual = stepper.XACTUAL();
        if (motorCalibration)
        {
            if (motorEnabled)
            {
                if (stepper.stallguard())
                {
                    Serial.println("Motor stalled.");
                    motorStallTime = now;
                    motorEnabled = false;
                    setCalRotFraction();
                }
                else
                {
                    int32_t minPos = motorStartPosition - (1 + calRotStartFraction) * positionFromRotations(motorConfig.CAL_ROT);
                    int32_t maxPos = motorStartPosition + (1 - calRotStartFraction) * positionFromRotations(motorConfig.CAL_ROT);
                    if (stepperTargetPosition != minPos && stepperTargetPosition != maxPos)
                    {
                        stepperTargetPosition = minPos;
                    }

                    if (stepper.position_reached())
                    {

                        stepperTargetPosition = stepperPositionActual > motorStartPosition ? minPos : maxPos;
                    }

                    stepper.VMAX(velocityFromRPM(motorConfig.VMAX_RPM));
                    stepper.RAMPMODE(positioning);
                    stepper.XTARGET(stepperTargetPosition);

                    // Serial.printf(
                    //     "TOFF: %2d, Pos: %12d, Target: %12d, Vel: %12d, RPM: %5.1f, TSTEP: %7d, CS: %2d, GSTAT: %d%d%d, SG: %4d, FS: %1d\n",
                    //     stepper.toff(),
                    //     stepper.XACTUAL(),
                    //     stepper.XTARGET(),
                    //     stepper.VACTUAL(),
                    //     stepperRPMActual,
                    //     stepper.TSTEP(),
                    //     stepperCurrentScale,
                    //     stepper.reset(), stepper.drv_err(), stepper.uv_cp(),
                    //     stepperStallguardResult,
                    //     stepper.fsactive());
                }
            }
            else if (!motorEnabled && now - motorStallTime > 5e6)
            {
                restartStepper();
                motorEnabled = true;
            }
        }
        else
        {
            float absTargetRPM = abs(stepperTargetRPM);

            float constrainedRPMTarget = constrain(absTargetRPM, 0.0, motorConfig.VMAX_RPM);

            stepperVMax = velocityFromRPM(constrainedRPMTarget);

            stepperRampMode = stepperTargetRPM < 0 ? negative : positive;

            stepper.VMAX(motorEnabled ? stepperVMax : 0);
            stepper.RAMPMODE(stepperRampMode);

            if (motorEnabled)
            {
                if (stepper.stallguard())
                {
                    Serial.println("Motor stalled!");
                    motorEnabled = false;
                    setCalRotFraction();
                    char message[26] = "{\"motor_stalled\": true}";
                    sendToProgram(message, sizeof(message));
                }
            }
            else
            {
                stepper.VMAX(0);
            }
        }
    }
}
