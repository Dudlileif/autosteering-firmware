#include "Sensors.h"
#include "StepperMotor/StepperMotor.h"

unsigned long sensorPrevUpdateTime = 0;

Adafruit_BNO08x_RVC imuRVC;

Adafruit_ADS1115 ads1115;

BNO08x_RVC_Data currentImuReading;
BNO08x_RVC_Data prevImuReading;

int16_t wasReading = 0;

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
    ads1115.begin(ADS1X15_ADDRESS, &Wire1);
    ads1115.setGain(GAIN_ONE);
    ads1115.setDataRate(RATE_ADS1115_860SPS);
    ads1115.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, false);

    Serial.println("WAS initialized.");
}

void updateImuReading()
{
    if (!imuRVC.read(&currentImuReading))
    {
        return;
    }
}

StaticJsonDocument<SENSOR_DATA_SIZE> getSensorData()
{
    StaticJsonDocument<SENSOR_DATA_SIZE> data;

    if (abs(prevImuReading.yaw - currentImuReading.yaw) < 20 && abs(prevImuReading.pitch - currentImuReading.pitch) < 20 && abs(prevImuReading.roll - currentImuReading.roll) < 20)
    {
        data["yaw"] = roundToNumberOfDecimals(currentImuReading.yaw, 3);
        data["pitch"] = roundToNumberOfDecimals(currentImuReading.pitch, 3);
        data["roll"] = roundToNumberOfDecimals(currentImuReading.roll, 3);
    }

    if (ads1115.conversionComplete())
    {
        wasReading = ads1115.getLastConversionResults();
        data["was"] = wasReading;
        ads1115.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, false);
    }

    data["motor_enabled"] = motorEnabled;
    if (abs(stepperRPMActual) > 0 || motorEnabled || motorCalibration)
    {
        data["motor_rpm"] = roundToNumberOfDecimals(stepperRPMActual, 3);
        data["motor_sg"] = stepperStallguardResult;
        data["motor_cs"] = stepperCurrentScale;
        if (motorCalibration)
        {
            data["motor_pos"] = roundToNumberOfDecimals((stepperPositionActual), 2);
            data["motor_target"] = roundToNumberOfDecimals(rotationsFromPosition(stepperTargetPosition), 2);
        }
    }

    prevImuReading.yaw = currentImuReading.yaw;
    prevImuReading.pitch = currentImuReading.pitch;
    prevImuReading.roll = currentImuReading.roll;

    return data;
}
