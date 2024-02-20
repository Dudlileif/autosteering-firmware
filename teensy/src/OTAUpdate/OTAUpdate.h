#ifndef OTAUPDATE_H
#define OTAUPDATE_H

#define FLASH_ID "fw_teensy41"

extern bool priorityMessageInProgress;
extern bool firmwareUpdateInProgress;

bool priorityModeEnabled();

void performUpdate();

#endif