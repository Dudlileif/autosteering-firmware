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

#ifndef OTAUPDATE_H
#define OTAUPDATE_H

#include <Arduino.h>

#define FORMAT_LITTLEFS_IF_FAILED true

extern bool priorityMessageInProgress;

extern bool doUpdate;

extern String updateFileName;
void mountFileSystem();

void attemptToUpdate();

bool attemptToUpdateSelf(const String &);

bool performUpdate(Stream &updateSource, size_t updateSize);

#ifdef AUTOSTEERING_BRIDGE

bool attemptToUpdateTeensy(const String &);

bool performTeensyUpdate(Stream &updateSource, size_t updateSize);
#endif

#endif