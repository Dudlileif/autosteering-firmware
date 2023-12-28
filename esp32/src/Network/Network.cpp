#include "Network.h"
#include "Config/Config.h"

WiFiConfig wifiConfig;

WiFiUDP sendUDP;
WiFiUDP receiveUDP;

IPAddress myIP;

WiFiMulti wifiMulti;

Destination destinations[4];
WiFiClient tcpClient;

wl_status_t prevWiFiClientStatus = WL_DISCONNECTED;

void setWiFiLED(int r, int g, int b)
{
    analogWrite(WIFI_LED_R, r);
    analogWrite(WIFI_LED_G, g);
    analogWrite(WIFI_LED_B, b);
}

void startWiFiAP()
{

    // Yellow LED
    setWiFiLED(127, 127, 0);

    WiFi.disconnect();
    Serial.println();
    Serial.println("Configuring access point...");

    // You can remove the password parameter if you want the AP to be open.
    // a valid password must have more than 7 characters
    if (!WiFi.softAP(String(wifiConfig.apSSID), String(wifiConfig.apPassword)))
    {
        log_e("Soft AP creation failed.");
        while (1)
            ;
    }
    myIP = WiFi.softAPIP();
    Serial.printf("AP IP address: %s\n", myIP.toString());
    if (!MDNS.begin(wifiConfig.hostname))
    {
        Serial.println("Error setting up MDNS responder!");
        while (1)
        {
            delay(1000);
        }
    }
    Serial.printf("ESP hostname: %s\n", wifiConfig.hostname);
}

void startWiFiClient()
{
    // Red LED
    setWiFiLED(127, 0, 0);

    if (strlen(wifiConfig.ssid[0]) == 0)
    {
        Serial.println("No WiFi client SSIDs set up, reverting to AP mode");
        return;
    }

    WiFi.softAPdisconnect();

    Serial.println("Known networks:");
    for (int i = 0; i < 5; i++)
    {
        if (strlen(wifiConfig.ssid[i]) > 0)
        {
            wifiMulti.addAP(wifiConfig.ssid[i], wifiConfig.password[i]);
            Serial.printf("\t%s\n", wifiConfig.ssid[i]);
        }
    }
    WiFi.scanNetworks();

    wifiMulti.run();
    if (!MDNS.begin(wifiConfig.hostname))
    {
        Serial.println("Error setting up MDNS responder!");
        while (1)
        {
            delay(1000);
        }
    }
    Serial.printf("ESP hostname: %s\n", wifiConfig.hostname);
    myIP = WiFi.localIP();
}

void checkWiFiStatus()
{
    if (WiFi.getMode() == WIFI_MODE_STA)
    {
        if (wifiMulti.run(5000) != WL_CONNECTED)
        {
            Serial.println("No network found, starting AP mode.");
            startWiFiAP();
        }

        wl_status_t status = WiFi.status();
        if (status == WL_CONNECTED)
        {
            setWiFiLED(0, 127, 0);
            if (status != prevWiFiClientStatus)
            {
                Serial.printf("WiFi connected to: %s\n", WiFi.SSID());
                Serial.printf("IP address: %s\n", WiFi.localIP().toString());
            }
        }
        else
        {
            setWiFiLED(127, 0, 0);
            if (status != prevWiFiClientStatus)
            {
                Serial.println("WiFi disconnected");
            }
        }
        prevWiFiClientStatus = status;
    }

    else if (WiFi.getMode() == WIFI_MODE_AP)
    {
        static uint prevConnected = 0;
        uint connected = WiFi.softAPgetStationNum();
        if (connected < 1)
        {
            // Pink LED
            setWiFiLED(63, 127, 63);
        }
        else
        {
            // Blue LED
            setWiFiLED(0, 0, 127);
        }
        if (prevConnected != connected)
        {
            Serial.printf("Connected devices: %d", connected);
            Serial.println();
            prevConnected = connected;

            if (connected < 1)
            {
                // Yellow LED
                setWiFiLED(127, 127, 0);
            }
            else
            {
                // Blue LED
                setWiFiLED(0, 0, 127);
            }
        }
    }
}

int findDestinationIndexToOverwrite()
{
    int freeIndex = 0;
    for (int i = 1; i < 4; i++)
    {
        if (destinations[i].heartbeat == 0)
        {
            freeIndex = i;
            break;
        }
        else if (destinations[i].heartbeat < destinations[freeIndex].heartbeat)
        {
            freeIndex = i;
        }
    }
    return freeIndex;
}

void updateDestinations()
{
    Destination dest = Destination();
    dest.ip = receiveUDP.remoteIP();
    dest.heartbeat = millis();

    for (int i = 0; i < 4; i++)
    {
        if (destinations[i].ip == dest.ip)
        {
            // Already have this in our system, update
            // the heartbeat time
            destinations[i] = dest;
            return;
        }
    }

    int indexToReplace = findDestinationIndexToOverwrite();
    destinations[indexToReplace] = dest;
}

bool checkHeartbeats()
{
    bool clientsAlive = false;
    for (int i = 0; i < 4; i++)
    {
        if (destinations[i].heartbeat != 0)
        {
            if (millis() - HEARTBEAT_BUFFER > destinations[i].heartbeat)
            {
                destinations[i].heartbeat = 0;
            }
            else
            {
                clientsAlive = true;
            }
        }
    }
    return clientsAlive;
}

void sendUdpPacket(uint8_t *data, int packetSize, IPAddress destinationIP, uint destinationPort)
{
    digitalWrite(SEND_LED_PIN, HIGH);

    sendUDP.beginPacket(destinationIP, destinationPort);
    sendUDP.write(data, packetSize);
    sendUDP.endPacket();

    digitalWrite(SEND_LED_PIN, LOW);
}

void sendUdpData(uint8_t *buffer, int messageSize)
{
    for (int i = 0; i < 4; i++)
    {
        if (destinations[i].heartbeat != 0)
        {
            sendUdpPacket(buffer, messageSize, destinations[i].ip, wifiConfig.udpSendPort);
        }
    }
}

int receiveUdpPacket(char *udpPacketBuffer)
{
    int size = receiveUDP.parsePacket();
    if (size > 0)
    {
        digitalWrite(LED_BUILTIN, HIGH);

        size = receiveUDP.read(udpPacketBuffer, size);

        updateDestinations();

        if (strstr(udpPacketBuffer, "Use me as Ntrip server!"))
        {
            IPAddress sender = receiveUDP.remoteIP();
            tcpClient.stop();
            tcpClient.connect(sender, wifiConfig.tcpSendPort);
            tcpClient.print("Connection from ESP32.");
            Serial.print("Connected to TCP server: ");
            Serial.print(sender);
            Serial.print(":");
            Serial.println(wifiConfig.tcpSendPort);
        }

        digitalWrite(LED_BUILTIN, LOW);
    }
    return size;
}

int receiveTcpPacket(uint8_t *tcpPacketBuffer)
{
    int size = tcpClient.available();
    if (size > 0)
    {
        digitalWrite(LED_BUILTIN, HIGH);

        size = tcpClient.read(tcpPacketBuffer, size);

        digitalWrite(LED_BUILTIN, LOW);
    }
    return size;
}

IPAddress getIPAddress()
{
    if (WiFi.getMode() == WIFI_MODE_AP)
    {
        return WiFi.softAPIP();
    }
    return WiFi.localIP();
}