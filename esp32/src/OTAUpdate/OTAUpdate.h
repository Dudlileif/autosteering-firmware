#ifndef OTAUPDATE_H
#define OTAUPDATE_H

#include <Arduino.h>

#define FORMAT_LITTLEFS_IF_FAILED true

extern String teensyFirmwareVersion;

extern bool priorityMessageInProgress;

extern bool doUpdate;

extern String updateFileName;
void mountFileSystem();

void attemptToUpdate();

bool attemptToUpdateSelf(const String &);

bool attemptToUpdateTeensy(const String &);

bool performUpdate(Stream &updateSource, size_t updateSize);

bool performTeensyUpdate(Stream &updateSource, size_t updateSize);

#endif