#ifndef STEPPERMOTOR_H
#define STEPPERMOTOR_H

#include <Arduino.h>
#include <TMCStepper.h>
#include <ArduinoJson.h>

#include <MotorConfig.h>

extern MotorConfig motorConfig;

extern bool motorEnabled;
extern bool motorCalibration;

enum StepperRampMode
{
    positioning,
    positive,
    negative,
    hold,
};

extern float stepperTargetRPM;
extern int32_t stepperTargetPosition;
extern StepperRampMode stepperRampMode;
extern uint32_t stepperVMax;

extern float stepperRPMActual;
extern int32_t stepperPositionActual;
extern uint16_t stepperStallguardResult;
extern uint8_t stepperCurrentScale;

extern unsigned long stepperPrevUpdateTime;
extern unsigned long stepperPrevCommandTime;

extern TMC5160Stepper stepper;

void stepperInit();

int32_t positionFromRotations(float rotations);

float rotationsFromPosition(int32_t position);

uint32_t velocityFromRPM(float rpm);

float rpmFromVelocity(int32_t velocity);

uint32_t tFromVelocity(uint32_t velocity);

uint32_t tFromRPM(float rpm);

uint32_t velocityFromT(uint32_t t);

float rpmFromT(uint32_t t);

uint32_t accelerationFromRPMS2(float rpms2);

float rpms2FromAcceleration(uint32_t acceleration);

void restartStepper();

void handleMotorControls(DynamicJsonDocument &document);

void updateStepper();

#endif