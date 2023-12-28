#include "OTAUpdate.h"

#include <Arduino.h>

#include <MultiStream.h>
#include <FXUtil.h> // read_ascii_line(), hex file support
extern "C"
{
#include <FlashTxx.h> // TLC/T3x/T4x/TMM flash primitives
}

#include "Config/Config.h"

const char *firmwareDate = "2023-12-28";

bool priorityMessageInProgress = false;
bool firmwareUpdateInProgress = false;

bool priorityModeEnabled()
{
    return digitalRead(PRIORITY_MESSAGE_SIGNAL_PIN) || priorityMessageInProgress || firmwareUpdateInProgress;
}

void performUpdate()
{
    MultiStream multiSerial = MultiStream(&Serial, &NETWORK_SERIAL);
    // Prepare buffer
    uint32_t bufferAddr, bufferSize;
    if (firmware_buffer_init(&bufferAddr, &bufferSize) == 0)
    {
        multiSerial.println("Unable to create firmware update buffer");
    }
    multiSerial.printf("Created buffer = %1luK %s (%08lX - %08lX)\n",
                       bufferSize / 1024, IN_FLASH(bufferAddr) ? "FLASH" : "RAM",
                       bufferAddr, bufferAddr + bufferSize);

    bool updateReady = false;
    uint32_t waitStart = millis();
    multiSerial.println("Teensy waiting for update on serial");
    while (!updateReady && millis() - waitStart < 2000)
    {
        if (NETWORK_SERIAL.available())
        {
            multiSerial.println("Teensy found update on serial");
            updateReady = true;
            break;
        }
    }

    if (updateReady)
    { // Perform update
        update_firmware(&NETWORK_SERIAL, &multiSerial, bufferAddr, bufferSize);
    }
    // If the update fails, we erase the buffer and reboot.
    multiSerial.println("Update failed.");
    multiSerial.printf("Erasing FLASH buffer / free RAM buffer...\n");
    multiSerial.~MultiStream();
    firmware_buffer_free(bufferAddr, bufferSize);
    REBOOT;
}
