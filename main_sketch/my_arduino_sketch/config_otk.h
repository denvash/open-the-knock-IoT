#ifndef CONFIG_AZURE_H
#define CONFIG_AZURE_H


#define MESSAGE_MAX_LEN 1024

/**
 * Find under Microsovift Azure IoT Suite -> DEVICES -> <your device> -> Device Details and Authentication Keys
 * String containing Hostname, Device Id & Device Key in the format:
 *  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"    
 */
#define IOT_CONFIG_CONNECTION_STRING  "HostName=otk-iot.azure-devices.net;DeviceId=myKnockDevice;SharedAccessKey=uFI6R4YG92iv0VA0O02Z7ostx+RQcJOL4DXIVzS/61s="
//#define IOT_CONFIG_CONNECTION_STRING	"HostName=otk-iot.azure-devices.net;DeviceId=myKnockDevice;SharedAccessKey=M3MUPoTcyUj7YHr2hLo24ytn6WG/OaDRGuVEDxDZIbc="
 /** 
 * Choose the transport protocol
 */
#define IOT_CONFIG_MQTT              // uncomment this line for MQTT
//#define IOT_CONFIG_HTTP              	// uncomment this line for HTTP
#define MAX_ID 5						// number of family users
#define MAX_SINGLE_KNOCK 5				// maximum number of knocks in a single digit
#define MAX_ID_SEQUENCE_LENGTH 1		// maximum number of digits in ID sequence
#define MAX_KNOCKS 20					// maximum total knocks
#define MAX_SEQUENCE_LENGTH 10			// maximum digits in sequence
#define MAX_SEQUENCE_STR (2 * MAX_SEQUENCE_LENGTH - 1)
#define SEQUENCE_INTERVAL_THRESHOLD 60 	// percent of longest intervall that will count as part of same digit in sequence
#define MAX_TRIALS 3					// maximum number of trials for challenge
#define NO_ID 255
#define OVERRIDE_ID 254
#define MAX_PENDING_TIME 10000			// in milliseconds

#define DIGIT_TO_KNOCKS(digit) (((digit) % MAX_SINGLE_KNOCK) + 1)

typedef enum {
	eLOOP_STATE_IDENTIFY = 0,
	eLOOP_STATE_PENDING,
	eLOOP_STATE_CHALLENGE_READY,
	eLOOP_STATE_CHALLENGE,
	eLOOP_STATE_OPEN,

	eLOOP_STATE_NUM_OF
} config_eLoopState;

typedef enum {
	eCHALLENGE_MATRIX = 0,
	eCHALLENGE_MENU,
	eCHALLENGE_KEYWORD,

	eCHALLENGE_NUM_OF
} config_eChallenge;

								   // Tuning constants.  Could be made vars and hoooked to potentiometers for soft configuration, etc.

const int threshold = 7;           // Minimum signal from the piezo to register as a knock
const int rejectValue = 25;        // If an individual knock is off by this percentage of a knock we don't unlock..
const int averageRejectValue = 15; // If the average timing of the knocks is off by this percent we don't unlock.
const int knockFadeTime = 180;     // milliseconds we allow a knock to fade before we listen for another one. (Debounce timer.)
const int lockTurnTime = 650;      // milliseconds that we run the motor to get it to go a half turn.
const int errorThreshold = 1;		// number of acceptable errors in sequence check
const int maximumKnocks = 20;       // Maximum number of knocks to listen for.

const int knockComplete = 3000;     // Longest time to wait for a knock before we assume that it's finished.

									// Variables.
int secretCode[maximumKnocks] = { 50, 25, 25, 50, 100, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  // Initial setup: "Shave and a Hair Cut, two bits."
int knockSensorValue = 0;           // Last reading of the knock sensor.
int programButtonPressed = false;   // Flag so we remember the programming button setting at the end of the cycle.


#endif /* CONFIG_AZURE_H */