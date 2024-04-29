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

#include <main.h>
#include <Config.h>
#include <Network.h>
#include <ArduinoJson.h>

const int minButtonTriggerPeriod = 250; // Milliseconds

const int minSendPeriod = 100; // Milliseconds

const int buttonTimeout = 1000; // Milliseconds, resets button if this much time has passed without
                                // receiving confirmation.

uint32_t lastButtonStateSendTime = 0;

uint32_t lastButtonStateReceiveTime = 0;

const int numButtons = 3;

struct Button
{
    const uint8_t pin;
    const uint8_t ledPin;
    bool pressed;
    bool released;
    bool receivedState;
    bool ledState;
    uint32_t lastTriggerTime;
};

volatile Button buttons[] = {{REMOTE_BUTTON_1, REMOTE_STATE_LED_1, false, true, false, LOW, 0},
                             {REMOTE_BUTTON_2, REMOTE_STATE_LED_2, false, true, false, LOW, 0},
                             {REMOTE_BUTTON_3, REMOTE_STATE_LED_3, false, true, false, LOW, 0}};

void IRAM_ATTR button1Trigger()
{
    if (digitalRead(buttons[0].pin))
    {
        buttons[0].released = true;
    }
    else
    {
        uint32_t now = millis();
        if (now - buttons[0].lastTriggerTime > minButtonTriggerPeriod)
        {
            buttons[0].pressed = true;
            buttons[0].released = false;
            buttons[0].lastTriggerTime = now;
        }
    }
}
void IRAM_ATTR button2Trigger()
{
    if (digitalRead(buttons[1].pin))
    {
        buttons[1].released = true;
    }
    else
    {
        uint32_t now = millis();
        if (now - buttons[1].lastTriggerTime > minButtonTriggerPeriod)
        {
            buttons[1].pressed = true;
            buttons[1].released = false;
            buttons[1].lastTriggerTime = now;
        }
    }
}
void IRAM_ATTR button3Trigger()
{
    if (digitalRead(buttons[2].pin))
    {
        buttons[2].released = true;
    }
    else
    {
        uint32_t now = millis();
        if (now - buttons[2].lastTriggerTime > minButtonTriggerPeriod)
        {
            buttons[2].pressed = true;
            buttons[2].released = false;
            buttons[2].lastTriggerTime = now;
        }
    }
}

void sendButtonStates()
{
    JsonDocument json;
    JsonArray states = json["button_states"].to<JsonArray>();
    for (int i = 0; i < numButtons; i++)
    {
        states.add(buttons[i].pressed);
    }
    String data;
    serializeJson(json, data);
    sendUdpData(data.c_str(), data.length());
}

void receiveUdpMessage()
{ // UDP maxes out at 1460
    char udpPacketBuffer[1460];

    int udpPacketSize = receiveUdpPacket(udpPacketBuffer);
    if (udpPacketSize > 0)
    {
        sendUdpData("Remote control: Heartbeat", 26);
        if (udpPacketBuffer[0] == '{' && udpPacketBuffer[udpPacketSize - 1] == '}')
        {
            JsonDocument message;
            deserializeJson(message, udpPacketBuffer);
            if (message.containsKey("button_states"))
            {
                JsonArray buttonStates = message["button_states"];
                for (int i = 0; i < buttonStates.size(); i++)
                {
                    buttons[i].receivedState = buttonStates[i];
                }
                lastButtonStateReceiveTime = millis();
            }
            if (message.containsKey("remote_states"))
            {
                JsonArray remoteStates = message["remote_states"];
                for (int i = 0; i < remoteStates.size(); i++)
                {
                    buttons[i].ledState = remoteStates[i];
                }
            }
        }
    }
}

void setup()
{
    for (int i = 0; i < numButtons; i++)
    {
        pinMode(buttons[i].pin, INPUT_PULLUP);
        pinMode(buttons[i].ledPin, OUTPUT);
    }
    attachInterrupt(buttons[0].pin, button1Trigger, CHANGE);
    attachInterrupt(buttons[1].pin, button2Trigger, CHANGE);
    attachInterrupt(buttons[2].pin, button3Trigger, CHANGE);

    mainSetup();
}

void loop()
{
    if (mainLoop())
    {
        for (int i = 0; i < numButtons; i++)
        {
            digitalWrite(buttons[i].ledPin, buttons[i].ledState);
        }
        uint32_t now = millis();
        if (now - lastButtonStateSendTime > minSendPeriod)
        {
            lastButtonStateSendTime = now;
            bool sendState = false;
            for (int i = 0; i < numButtons; i++)
            {
                if (buttons[i].pressed != buttons[i].receivedState)
                {
                    sendState = true;
                }
            }
            if (sendState)
            {
                sendButtonStates();
            }

            for (int i = 0; i < numButtons; i++)
            {
                if ((!sendState && buttons[i].released) || now - buttons[i].lastTriggerTime > buttonTimeout)
                {
                    buttons[i].pressed = false;
                    buttons[i].receivedState = false;
                }
            }
        }
        receiveUdpMessage();
    }
}
#endif