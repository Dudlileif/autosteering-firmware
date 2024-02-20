#include "OTAUpdate.h"

#include <Arduino.h>

#include <MultiStream.h>
#include <FXUtil.h> // read_ascii_line(), hex file support
extern "C"
{
#include <FlashTxx.h> // TLC/T3x/T4x/TMM flash primitives
}

#include "Config/Config.h"
#include "Comms/Comms.h"

bool priorityMessageInProgress = false;
bool firmwareUpdateInProgress = false;

bool priorityModeEnabled()
{
    return digitalRead(PRIORITY_MESSAGE_SIGNAL_PIN) || priorityMessageInProgress || firmwareUpdateInProgress;
}

void performUpdate()
{
    // Prepare buffer
    uint32_t bufferAddr, bufferSize;
    if (firmware_buffer_init(&bufferAddr, &bufferSize) == 0)
    {
        usbAndNetworkSerial.println("Unable to create firmware update buffer");
    }
    usbAndNetworkSerial.printf("Created buffer = %1luK %s (%08lX - %08lX)\n",
                               bufferSize / 1024, IN_FLASH(bufferAddr) ? "FLASH" : "RAM",
                               bufferAddr, bufferAddr + bufferSize);

    bool updateReady = false;
    elapsedMillis waitingTime;
    usbAndNetworkSerial.println("Teensy waiting for update on serial");
    while (!updateReady && waitingTime < 2000)
    {
        if (NETWORK_SERIAL.available())
        {
            usbAndNetworkSerial.println("Teensy found update on serial");
            updateReady = true;
            break;
        }
    }
    if (updateReady)
    { // Perform update
        update_firmware(&NETWORK_SERIAL, &usbAndNetworkSerial, bufferAddr, bufferSize);
    }
    // If the update fails, we erase the buffer and reboot.
    usbAndNetworkSerial.println("Update failed.");
    usbAndNetworkSerial.printf("Erasing FLASH buffer / free RAM buffer...\n");
    usbAndNetworkSerial.~MultiStream();
    firmware_buffer_free(bufferAddr, bufferSize);
    REBOOT;
}
