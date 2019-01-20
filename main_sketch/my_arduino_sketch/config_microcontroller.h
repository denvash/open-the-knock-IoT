
#ifndef CONFIG_MICROCONTROLLER_H
#define CONFIG_MICROCONTROLLER_H

// Physical device information for board and sensor
#define DEVICE_ID "SparkFun ESP8266 Thing Dev"
#define DHT_TYPE DHT22

// Pin layout configuration
#define LED_PIN 5
#define DHT_PIN 2

// If don't have a physical DHT sensor, can send simulated data to IoT hub
#define SIMULATED_DATA false

// EEPROM address configuration
#define EEPROM_SIZE 512

// SSID and SSID password's length should < 32 bytes
// http://serverfault.com/a/45509
#define SSID_LEN 32
#define PASS_LEN 32
#define CONNECTION_STRING_LEN 256
#define MAX_WIFI_WAIT_IN_SEC 15
#define MAX_WIFI_WAIT (MAX_WIFI_WAIT_IN_SEC * 1000)
/**
 * WiFi setup
 */
//#define IOT_CONFIG_WIFI_SSID            "HUAWEI P9"
//#define IOT_CONFIG_WIFI_PASSWORD        "bcc27a78108a"
#define IOT_CONFIG_WIFI_SSID            "tpLink_machine"
#define IOT_CONFIG_WIFI_PASSWORD        "0547692556"

// Pin definitions
const int knockSensor = A0;         // Piezo sensor on pin 0.
const int programSwitch = 2;       // If this is high we program a new code.
const int lockMotor = D6;           // Gear motor used to turn the lock.
//const int redLED = D0;              // Status LED
//const int greenLED = D0;            // Status LED
const int redAlert = D0;
const int waterSplash = D1;

#endif /* CONFIG_MICROCONTROLLER_H */
