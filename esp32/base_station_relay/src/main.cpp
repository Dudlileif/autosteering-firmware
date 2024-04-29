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

#ifndef _MAIN_CPP
#define _MAIN_CPP

#include "main.h"
#include "GnssComms.h"
#include <Config.h>

void setup()
{
    pinMode(GNSS_READ_FAIL_LED_PIN, OUTPUT);
    mainSetup();
    GNSS_SERIAL.setRxBufferSize(1024);
    GNSS_SERIAL.begin(115200, SERIAL_8N1, RXD2, TXD2);
}

void loop()
{
    if (mainLoop())
    {
        receiveGNSSData();
    }
}
#endif