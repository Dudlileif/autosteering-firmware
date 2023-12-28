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
unsigned long stepperLastCommandTime = 0;

const uint32_t f_clk = 12000000;
// On TMC5160 BOB          CSN   (r_sense, ohm)  SDI           SDO           SCK    (link_address)
TMC5160Stepper stepper = TMC5160Stepper(PIN_SPI_SS1, 0.075, PIN_SPI_MOSI1, PIN_SPI_MISO1, PIN_SPI_SCK1, -1);

void stepperInit()
{
    stepper.begin();                                      // Start SPI connection with driver
    stepper.push();                                       // Not sure if needed
    stepper.toff(motorConfig.TOFF);                       // Enable the driver with toff > 0
    stepper.microsteps(motorConfig.MICRO_STEPS);          // Set MICRO_STEPS
    stepper.hold_multiplier(motorConfig.hold_multiplier); // 0..1, multiplier to get IHOLD from IRUN.
    stepper.rms_current(motorConfig.RMS_CURRENT);         // Set the motor current (mA)
    stepper.IHOLD_IRUN(motorConfig.IHOLD_IRUN);           // 0..31, Current scale when motor is running. IRUN=IHOLD_IRUN, IHOLD = hold_multiplier*IHOLD_IRUN.
    stepper.iholddelay(motorConfig.IHOLDDELAY);           // 0..15, IHOLDDELAY * 2^18 clock cycles after TZEROWAIT per current decrement from run to hold, 0 means instant hold
    stepper.freewheel(motorConfig.freewheel);             // Stand still option when motor current is zero, 0: normal op., 1: freewheeling, 2: coil shorted with LS drivers,
                                                          // 3: coil shorted with HS driver
    stepper.tbl(motorConfig.TBL);                         // 0..3, Comparator blank time, 0: 16 t_clk, 1: 24 t_clk, 2: 36 t_clk, 3: 54 t_clk
    stepper.hstrt(motorConfig.HSTRT);                     // 0...15 -> -3...12, Hysteresis start value added to HEND.
    stepper.hend(motorConfig.HEND);                       // 0...15 -> -3...12, Hysteresis low value.
    // Sets the delay time after stand still until the motor current powers
    // down.
    // 0..255 * 2^18*t_clk
    stepper.TPOWERDOWN(motorConfig.TPOWERDOWN);

    // Sets the wait time at zero velocity after a ramp before continuing
    // to the other side of zero.
    // 0..65535 * 512*t_clk
    stepper.TZEROWAIT(motorConfig.TZEROWAIT);

    // StealthChop
    stepper.en_pwm_mode(motorConfig.en_pwm_mode);     // Enable StealthChop when T > TPWMTHRS
    stepper.pwm_autoscale(motorConfig.pwm_autoscale); // Enable automatic current control, PWM amplitude scaling
    stepper.pwm_autograd(true);                       // Enable automatic PWM gradient adaptation

    // Stallguard
    stepper.sfilt(motorConfig.sfilt); // Require 4 fullsteps for measurement
    stepper.sgt(motorConfig.SGT);     // 32 , range -64 -> 63 (high to low sensitivity)
    stepper.sg_stop(motorConfig.sg_stop);

    // SpreadCycle
    stepper.TPWMTHRS(motorConfig.TPWMTHRS); // Switch to SpreadCycle when T < TPWMTHRS
    stepper.chm(motorConfig.chm);

    // CoolStep
    stepper.TCOOLTHRS(motorConfig.TCOOLTHRS); // Enable CoolStep when T < 120
    stepper.semin(motorConfig.semin);         // If sg_result < semin*32, current is increased
    stepper.semax(motorConfig.semax);         // If sg_result > (semin+semax+1)*32, current is decreased

    // DC step
    stepper.THIGH(motorConfig.THIGH);       // Enable DcStep when T < 100, disables CoolStep and StallGuard
    stepper.vhighfs(motorConfig.vhighfs);   // Enable fullstep at high velocities
    stepper.vhighchm(motorConfig.vhighchm); // Switches to chm(1) and fd(0) when exceeding VHIGH
    stepper.VDCMIN(motorConfig.VDCMIN);
    stepper.dc_time(motorConfig.DC_TIME);
    stepper.dc_sg(motorConfig.DC_SG);

    Serial.println("Stepper initialized.");
}

int32_t positionFromRotations(float rotations)
{
    return round(rotations * motorConfig.MICRO_STEPS * motorConfig.STEPS_PER_ROT);
}

float rotationsFromPosition(int32_t position)
{
    return position / motorConfig.MICRO_STEPS / motorConfig.STEPS_PER_ROT;
}

uint32_t velocityFromRPM(float rpm)
{
    return round(rpm / 60.0 * motorConfig.MICRO_STEPS * motorConfig.STEPS_PER_ROT);
}

float rpmFromVelocity(uint32_t velocity)
{
    return velocity * 60 / motorConfig.MICRO_STEPS / motorConfig.STEPS_PER_ROT;
}

uint32_t tFromVelocity(uint32_t velocity)
{
    return pow(2, 24) / (velocity * 256 / motorConfig.MICRO_STEPS);
}

uint32_t tFromRPM(float rpm)
{
    return pow(2, 24) / (velocityFromRPM(rpm) * 256 / motorConfig.MICRO_STEPS);
}

void restartStepper()
{
    stepper.sg_stop(0);
    stepper.sg_stop(motorConfig.sg_stop);
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
        stepperLastCommandTime = micros();
    }
    else if (!calibrate.isNull())
    {
        motorCalibration = calibrate;
        stepperLastCommandTime = micros();
    }
    if (!enableMotor.isNull())
    {
        if (motorEnabled != enableMotor)
        {
            restartStepper();
        }
        motorEnabled = enableMotor;
    }
}

void updateStepper()
{
    unsigned long now = micros();
    if (now - stepperLastCommandTime > STEPPER_COMMAND_UPDATE_US)
    {
        if (motorEnabled)
        {
            stepperVMax = 0;
            stepperTargetRPM = 0;
            motorEnabled = false;
            Serial.println("Motor stopped, too long since last command.");

            char message[29] = "{\"motor_no_command\": true}";
            sendToProgram(message, sizeof(message));
        }
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
                {
                    int32_t absTargetPosition = positionFromRotations(motorConfig.CAL_ROT);
                    if (abs(stepperTargetPosition) != absTargetPosition)
                    {
                        stepperTargetPosition = absTargetPosition;
                    }

                    if (stepperPositionActual == stepperTargetPosition && stepperTargetPosition != 0)
                    {
                        stepperTargetPosition *= -1;
                    }

                    stepper.AMAX(motorConfig.AMAX);
                    stepper.DMAX(motorConfig.AMAX);
                    stepper.VMAX(velocityFromRPM(motorConfig.RPMMAX));
                    stepper.v1(0);
                    stepper.VSTOP(10);
                    stepper.RAMPMODE(positioning);
                    stepper.XTARGET(stepperTargetPosition);
                }
            }
            else
            {
                stepper.RAMPMODE(hold);
            }
        }
        else
        {
            float absTargetRPM = abs(stepperTargetRPM);

            float constrainedRPMTarget = constrain(absTargetRPM, 0.0, motorConfig.RPMMAX);

            stepperVMax = velocityFromRPM(constrainedRPMTarget);

            stepperRampMode = stepperTargetRPM < 0 ? negative : positive;

            stepper.AMAX(motorConfig.AMAX);
            stepper.VMAX(motorEnabled ? stepperVMax : 0);
            stepper.VSTOP(10);
            stepper.RAMPMODE(stepperRampMode);
            // Serial.printf("RPM_MAX:%f, RPM:%f\n",
            //               stepper.VMAX() / MICRO_STEPS / STEPS_PER_ROT * 60.0 * (stepperDirection == 1 ? 1 : -1),
            //               stepperRPMActual);

            if (motorEnabled)
            {
                if (stepper.stallguard())
                {
                    Serial.println("Motor stalled!");
                    motorEnabled = false;
                    char message[26] = "{\"motor_stalled\": true}";
                    sendToProgram(message, sizeof(message));
                }
                else
                {
                    // Serial.printf("{FS: %d, SG: %d, T: %d, V: %d, RPM: %3.1f}\n",
                    //               stepper.fsactive(),
                    //               stepper.sg_result(),
                    //               stepper.TSTEP(),
                    //               stepper.VACTUAL(),
                    //               stepperRPMActual);
                }
            }
        }
    }
}
