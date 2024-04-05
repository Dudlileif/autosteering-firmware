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

#include "PidController.h"

float PidController::next(float error)
{
    float output = 0.0;

    float timeInSeconds = time / 1e6;

    // P, proportional gain applied to the error.
    output = p * error;

    // I, add the new error value to the integral/average.
    integral =
        (integral * ((integralSize - 1) / integralSize)) + timeInSeconds * error / integralSize;

    output += i * integral;

    // D, the change since the last loop.
    float derivative = (error - prevError) / timeInSeconds;

    output += d * derivative;

    prevError = error;
    time = 0;
    return output;
}

void PidController::clear()
{
    prevError = 0;
    integral = 0;
    time = 0;
}