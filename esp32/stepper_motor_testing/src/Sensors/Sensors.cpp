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

#include "Sensors.h"
#include <Config/Config.h>

#include <SPI.h>

SPIClass *adcSPI = NULL;

static const int clk = 800000; // 0.8 MHz

void initSensors()
{
    adcSPI = new SPIClass(HSPI);
    adcSPI->begin();
    pinMode(PIN_SPI_SS2, OUTPUT);
}

uint16_t readADC()
{

    adcSPI->beginTransaction(SPISettings(clk, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_SPI_SS2, LOW);
    uint8_t readingMSB = adcSPI->transfer(0) & 0b00011111;
    uint8_t readingLSB = adcSPI->transfer(0) & 0b11111110;
    uint16_t value = (readingMSB << 7) + (readingLSB >> 1);
    digitalWrite(PIN_SPI_SS2, HIGH);
    adcSPI->endTransaction();
    // Serial.printf("%5d | %5d, %5d | %5d, %5d\n", value, readingMSB, readingLSB, readingMSB << 7, readingLSB >> 1);
    return value;
}