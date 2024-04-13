#include "GnssComms.h"
#include <../Config/Config.h>
#include <../Network/Network.h>

const int N = 4096;
uint8_t ubxBuffer[N];
size_t ubxSize = 0;
int16_t zeroIndex = 0;

void receiveGNSSData()
{
    while (GNSS_SERIAL.available())
    {
        ubxBuffer[(zeroIndex + ubxSize) % N] = GNSS_SERIAL.read();
        ubxSize++;
    }
    if (ubxSize > 0)
    {
        if (ubxBuffer[zeroIndex % N] != 0xb5)
        {
            Serial.printf("Header error pos 0: %d\n", ubxBuffer[zeroIndex % N]);
            ubxSize = 0;
            digitalWrite(GNSS_READ_FAIL_LED_PIN, HIGH);
            return;
        }
        if (ubxBuffer[(zeroIndex + 1) % N] != 0x62)
        {
            Serial.printf("Header error pos 1: %d\n", ubxBuffer[(zeroIndex + 1) % N]);
            ubxSize = 0;
            digitalWrite(GNSS_READ_FAIL_LED_PIN, HIGH);
            return;
        }

        if (ubxSize > 6)
        {
            int payloadSize = ubxBuffer[(zeroIndex + 4) % N] | ubxBuffer[(zeroIndex + 5) % N] << 8;
            if (ubxSize >= payloadSize + 8)
            {
                uint8_t ckA = 0, ckB = 0;

                for (int i = 2; i < payloadSize + 6; i++)
                {
                    ckA += ubxBuffer[(zeroIndex + i) % N];
                    ckB += ckA;
                }
                bool endsCorrectly = ckA == ubxBuffer[(zeroIndex + payloadSize + 6) % N] && ckB == ubxBuffer[(zeroIndex + payloadSize + 7) % N];
                if (endsCorrectly)
                {
                    digitalWrite(GNSS_READ_FAIL_LED_PIN, LOW);
                }

                if (!endsCorrectly)
                {
                    Serial.printf("CK_A:%d/%d, CK_B: %d/%d, Length:%d\n",
                                  ckA,
                                  ubxBuffer[(zeroIndex + payloadSize + 6) % N],
                                  ckB,
                                  ubxBuffer[(zeroIndex + payloadSize + 7) % N],
                                  payloadSize + 8);
                }

                if (endsCorrectly && WiFi.status() == WL_CONNECTED)
                {
                    uint8_t buffer[payloadSize + 8];
                    for (int i = 0; i < payloadSize + 8; i++)
                    {
                        buffer[i] = ubxBuffer[(zeroIndex + i) % N];
                    }
                    if (tcpClient != nullptr)
                    {
                        if (tcpClient->connected())
                        {
                            tcpClient->write((const char *)buffer, payloadSize + 8, ASYNC_WRITE_FLAG_COPY);
                        }
                    }

                    sendUdpPacket(buffer, payloadSize + 8, wifiConfig.rtkBaseStationAddress, wifiConfig.udpSendPort);
                }
                ubxSize -= payloadSize + 8;
                constrain(ubxSize, 0, N);
                zeroIndex = (zeroIndex + payloadSize + 8) % N;
            }
        }
        if (ubxSize >= N)
        {
            ubxSize = 0;
        }
    }
}
