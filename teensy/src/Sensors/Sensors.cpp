#include "Sensors.h"
#include "StepperMotor/StepperMotor.h"

unsigned long sensorPrevUpdateTime = 0;

Adafruit_BNO08x_RVC imuRVC;

Adafruit_ADS1115 ads1115;

unsigned long imuPrevUpdateTime = 0;

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
    if (motorEnabled || motorCalibration)
    {
        data["motor_rpm"] = roundToNumberOfDecimals(stepperRPMActual, 3);
        data["motor_sg"] = stepperStallguardResult;
        data["motor_cs"] = stepperCurrentScale;
        if (motorCalibration)
        {
            data["motor_pos"] = roundToNumberOfDecimals(rotationsFromPosition(stepperPositionActual), 2);
            data["motor_target"] = roundToNumberOfDecimals(rotationsFromPosition(stepperTargetPosition), 2);
        }
    }

    prevImuReading.yaw = currentImuReading.yaw;
    prevImuReading.pitch = currentImuReading.pitch;
    prevImuReading.roll = currentImuReading.roll;

    return data;
}
