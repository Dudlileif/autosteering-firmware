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

#include <ADC.h>
#include <ADC_util.h>
#include "Sensors.h"
#include "StepperMotor/StepperMotor.h"

elapsedMicros sensorPrevUpdateElapsedTime;

Adafruit_BNO08x_RVC imuRVC;

unsigned long imuPrevUpdateTime = 0;

BNO08x_RVC_Data currentImuReading;
BNO08x_RVC_Data prevImuReading;

uint16_t wasReading = 0;

ADC *adc = new ADC();

// Rounds a number to a certain number of decimals.
double roundToNumberOfDecimals(double value, int numDecimals)
{
    int sign = (value > 0) - (value < 0);

    return (int)(value * pow(10, numDecimals) - sign * 0.5) / (pow(10.0, numDecimals));
}

void imuInit()
{
    if (!imuRVC.begin(&IMU_SERIAL))
    {
        Serial.println("Failed to find BNO08x chip, please reboot.");
    }
    else
    {
        Serial.println("BNO08x found, IMU initialized.");
    }
}

void wasInit()
{
    adc->adc0->setAveraging(32);
    adc->adc0->setResolution(12);
    adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::MED_SPEED);
    adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED);
    adc->adc0->startContinuous(PIN_WAS);

    Serial.println("WAS initialized.");
}

// A copy of the function
// Adafruit_BNO08x_RVC::read(BNO08x_RVC_Data *heading)
// with direct use of serial, as using the original function
// caused crashes with pointers.
void updateImuReading()
{
    // Skip if no data available.
    if (!IMU_SERIAL.available())
    {
        return;
    }

    // Discard bytes until a correct starting byte appears.
    if (IMU_SERIAL.peek() != 0xAA)
    {
        IMU_SERIAL.read();
        return;
    }

    // Now read all 19 bytes
    if (IMU_SERIAL.available() < 19)
    {
        return;
    }
    // at this point we know there's at least 19 bytes available and the first
    if (IMU_SERIAL.read() != 0xAA)
    {
        // shouldn't happen baecause peek said it was 0xAA
        return;
    }
    // make sure the next byte is the second 0xAA
    if (IMU_SERIAL.read() != 0xAA)
    {
        return;
    }
    uint8_t buffer[19];
    // ok, we've got our header, read the actual data+crc
    if (!IMU_SERIAL.readBytes(buffer, 17))
    {
        return;
    };

    uint8_t sum = 0;
    // get checksum ready
    for (uint8_t i = 0; i < 16; i++)
    {
        sum += buffer[i];
    }
    if (sum != buffer[16])
    {
        return;
    }

    // The data comes in endian'd, this solves it so it works on all platforms
    int16_t buffer_16[6];

    for (uint8_t i = 0; i < 6; i++)
    {

        buffer_16[i] = (buffer[1 + (i * 2)]);
        buffer_16[i] += (buffer[1 + (i * 2) + 1] << 8);
    }
    currentImuReading.yaw = (float)buffer_16[0] * DEGREE_SCALE;
    currentImuReading.pitch = (float)buffer_16[1] * DEGREE_SCALE;
    currentImuReading.roll = (float)buffer_16[2] * DEGREE_SCALE;

    currentImuReading.x_accel = (float)buffer_16[3] * MILLI_G_TO_MS2;
    currentImuReading.y_accel = (float)buffer_16[4] * MILLI_G_TO_MS2;
    currentImuReading.z_accel = (float)buffer_16[5] * MILLI_G_TO_MS2;
}

JsonDocument getSensorData()
{
    JsonDocument data;

    // Only send IMU readings that are somewhat close to the previous
    if (abs(prevImuReading.yaw - currentImuReading.yaw) < 20 && abs(prevImuReading.pitch - currentImuReading.pitch) < 20 && abs(prevImuReading.roll - currentImuReading.roll) < 20)
    {
        data["yaw"] = roundToNumberOfDecimals(currentImuReading.yaw, 3);
        data["pitch"] = roundToNumberOfDecimals(currentImuReading.pitch, 3);
        data["roll"] = roundToNumberOfDecimals(currentImuReading.roll, 3);
        data["acc_x"] = roundToNumberOfDecimals(currentImuReading.x_accel, 3);
        data["acc_y"] = roundToNumberOfDecimals(currentImuReading.y_accel, 3);
        data["acc_z"] = roundToNumberOfDecimals(currentImuReading.z_accel, 3);
    }

    if (adc->adc0->isComplete())
    {
        wasReading = (uint16_t)adc->analogReadContinuous();
        data["was"] = wasReading;
    }

    data["motor_enabled"] = motorEnabled;
    if (motorEnabled || motorCalibration)
    {
        data["motor_rpm"] = roundToNumberOfDecimals(stepperRPMActual, 3);
        data["motor_sg"] = stepperStallguardResult;
        data["motor_cs"] = stepperCurrentScale;
        data["was_target"] = wasTarget;
    }
    prevImuReading = currentImuReading;

    return data;
}
