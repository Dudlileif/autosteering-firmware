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

#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>
#include <sys/types.h>

// If using ESP32-WROVER module, pins 6-11, 16 and 17 are already in use by the extended
// memory.

#define RXD2 2
#define TXD2 4

#define TEENSY_SERIAL Serial2 // Pins: RX: 2, TX: 4
#define GNSS_SERIAL Serial2   // Pins: RX: 2, TX: 4

#define FORMAT_LITTLEFS_IF_FAILED true

const int PRIORITY_MESSAGE_SIGNAL_PIN = 23;

const int WIFI_LED_R = 18;
const int WIFI_LED_G = 21;
const int WIFI_LED_B = 19;
const int SEND_LED_PIN = 5;

const int GNSS_READ_FAIL_LED_PIN = 27;

const uint SEND_LED_ON_MS = 250;

const int SERIAL_BAUD = 115200;

const uint HEARTBEAT_BUFFER_MS = 5000; // Milliseconds, UDP clients are dropped if not receiving a heartbeat within this time.

typedef struct color_t
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} color_t;

const color_t colorBlue = color_t{0, 0, 127};
const color_t colorGreen = color_t{0, 127, 0};
const color_t colorRed = color_t{127, 0, 0};
const color_t colorPink = color_t{63, 127, 63};
const color_t colorYellow = color_t{127, 127, 0};

#endif