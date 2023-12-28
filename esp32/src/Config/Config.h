#ifndef CONFIG_H
#define CONFIG_H
#include <sys/types.h>

#define RXD2 2
#define TXD2 4

#define TEENSY_SERIAL Serial2 // Pins: RX: 2, TX: 4

#define FORMAT_LITTLEFS_IF_FAILED true

#define LED_BUILTIN 13

const int PRIORITY_MESSAGE_SIGNAL_PIN = 23;

const int WIFI_TOGGLE_PIN = 32;
const int WIFI_LED_R = 18;
const int WIFI_LED_G = 21;
const int WIFI_LED_B = 19;
const int SEND_LED_PIN = 15;

const int SERIAL_BAUD = 115200;

const uint HEARTBEAT_BUFFER = 5000; // Milliseconds, UDP clients are dropped if not receiving a heartbeat within this time.

#endif