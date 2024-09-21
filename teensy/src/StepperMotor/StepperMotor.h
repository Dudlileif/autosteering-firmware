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

#ifndef STEPPERMOTOR_H
#define STEPPERMOTOR_H

#include <Arduino.h>
#include <TMCStepper.h>
#include <ArduinoJson.h>

#include <MotorConfig.h>
#include "PidController.h"

extern MotorConfig motorConfig;

extern PidController pidController;

extern bool motorEnabled;
extern bool motorCalibration;

enum StepperRampMode
{
    positioning,
    positive,
    negative,
    hold,
};

extern uint16_t wasTarget;
extern StepperRampMode stepperRampMode;
extern uint32_t stepperVMax;

extern float stepperRPMActual;
extern uint16_t stepperStallguardResult;
extern uint8_t stepperCurrentScale;

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

uint32_t accelerationFromRPMS(float rpms);

float rpmsFromAcceleration(uint32_t acceleration);

void restartStepper();

void handleMotorControls(JsonDocument &document);

void updateStepper();

#endif