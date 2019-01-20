#include <Servo.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>
#include "config_otk.h"
#include "config_microcontroller.h"

/* Variables */
IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
IOTHUB_CLIENT_STATUS clientStatus;
uint32_t safetyCount;
Servo myServo;
uint8_t currentId;

/* Externs */
extern uint8_t* currentChallengeSequence;
extern uint8_t matrixChallengeSequence[3];
extern String matrixChallengeStr;
extern uint8_t menuChallengeSequence[3];
extern String menuChallengeStr;
extern uint8_t passwordChallengeSequence[3];
extern String passwordChallengeStr;

/* Functions declaration */
void identifyRoutine();
void challengeRoutine();
void initSerial();
void initWifi();
void wifiTurnOff();
void initTime();
void Blink();
void(*resetFunc) (void) = 0; //declare reset function @ address 0

/*********************************/
/* 			Main code			 */
void setup() {
	safetyCount = millis();
	myServo.attach(lockMotor);
	myServo.write(0);
	delay(2000);
	pinMode(redAlert, OUTPUT);
	digitalWrite(redAlert, LOW);
	pinMode(waterSplash, OUTPUT);
	digitalWrite(waterSplash, LOW);
	//pinMode(greenLED, OUTPUT);
	//digitalWrite(greenLED, LOW);

	initSerial();
	//initServo(); TODO: reset servo
	//initWifi();
	initTime();
	/* wifi to low current */
	wifiTurnOff();
	/* wifi wake up */
	//initWifi();

	initScreen();
	drawWelcome();

	iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(IOT_CONFIG_CONNECTION_STRING, MQTT_Protocol);
    if (iotHubClientHandle == NULL)
    {
        Serial.println("Failed on IoTHubClient_CreateFromConnectionString.");
        while (1);
    }

    IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, receiveMessageCallback, NULL);
    IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, NULL);
    IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, twinCallback, NULL);
	Serial.println("starting listening");
}

config_eLoopState currentState = eLOOP_STATE_IDENTIFY;
void loop() {
	
	switch (currentState)
	{
		case eLOOP_STATE_IDENTIFY:
			identifyRoutine();
			//IoTHubClient_LL_DoWork(iotHubClientHandle);
			break;
		
		case eLOOP_STATE_PENDING:
			static int pendingStart = millis();
			if (millis() - pendingStart > MAX_PENDING_TIME)
			{
				drawCloudResponse("Cloud response error");
				delay(7000);
				IoTHubClient_LL_Destroy(iotHubClientHandle);
				resetFunc();
			}
			IoTHubClient_LL_DoWork(iotHubClientHandle);
			break;
		
		case eLOOP_STATE_CHALLENGE_READY:
			IoTHubClient_LL_DoWork(iotHubClientHandle);
			IoTHubClient_LL_GetSendStatus(iotHubClientHandle, &clientStatus);
			if (clientStatus == IOTHUB_CLIENT_SEND_STATUS_IDLE)
			{
				wifiTurnOff();
				Serial.println("state change to state challenge");
				currentState = eLOOP_STATE_CHALLENGE;
			}
			break;

		case eLOOP_STATE_CHALLENGE:
			challengeRoutine();
			/*if (safetyCount++ == 1000) {
				delay(3);
				yield();
				safetyCount = 0;
			}*/
			break;
		case eLOOP_STATE_OPEN:
			static bool firstEnter = true;
			if (firstEnter) 
			{
				IoTHubClient_LL_Destroy(iotHubClientHandle);
				wifiTurnOff();
				firstEnter = false;
			}
			// Listen for any knock at all.
			knockSensorValue = analogRead(knockSensor);
			if (safetyCount - millis() > 50) {
				//Serial.println(knockSensorValue);
				safetyCount = millis();
				//delay(3);
				yield();
				safetyCount = 0;
			}
			if (knockSensorValue >= threshold) {
				//Serial.println(knockSensorValue);
				uint8_t id = identifyKnock();
				if (id == currentId) 					
				{
					drawLocking();
					resetFunc();
				}
			}
			break;
		default:
			break;
	}
}


/*************************************************/
/* 			Function definitions	       	     */
void initSerial()
{
    // Start serial and initialize stdout
    Serial.begin(9600);
    Serial.setDebugOutput(true);
    Serial.println("Serial successfully inited.");
}

void initWifi()
{
	char* ssid = IOT_CONFIG_WIFI_SSID;
	char* pass = IOT_CONFIG_WIFI_PASSWORD;

	WiFi.forceSleepWake();
	delay( 1 );
	// Bring up the WiFi connection
	WiFi.mode( WIFI_STA );
	WiFi.setPhyMode(WIFI_PHY_MODE_11G);

    // Attempt to connect to Wifi network:
    Serial.printf("Attempting to connect to SSID: %s.\r\n", ssid);
    
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.begin(ssid, pass);
	delay(5000);
    while (WiFi.status() != WL_CONNECTED)
    {
		static int timerStart = millis();
		if (millis() - timerStart > MAX_WIFI_WAIT)
		{
			drawConnectingFailed();
			delay(5000);
			resetFunc();
		}
        // Get Mac Address and show it.
        // WiFi.macAddress(mac) save the mac address into a six length array, but the endian may be different. The huzzah board should
        // start from mac[0] to mac[5], but some other kinds of board run in the oppsite direction.
        uint8_t mac[6];
        WiFi.macAddress(mac);
        Serial.printf("You device with MAC address %02x:%02x:%02x:%02x:%02x:%02x connects to %s failed! Waiting 5 seconds to retry.\r\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], ssid);
        WiFi.begin(ssid, pass);
        delay(5000);
    }
    Serial.printf("Connected to wifi %s.\r\n", ssid);
}

void initTime()
{
    time_t epochTime;
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    while (true)
    {
        epochTime = time(NULL);

        if (epochTime == 0)
        {
            Serial.println("Fetching NTP epoch time failed! Waiting 2 seconds to retry.");
            delay(2000);
        }
        else
        {
            Serial.printf("Fetched NTP epoch time is: %lu.\r\n", epochTime);
            break;
        }
    }
}

void identifyRoutine()
{
	static uint8_t id = NO_ID;
	static int messageCount = 1;

	// Listen for any knock at all.
	knockSensorValue = analogRead(knockSensor);

	//drawAnalogRead(knockSensorValue);
	if (knockSensorValue >= threshold) {
		//Serial.println(knockSensorValue);
		id = identifyKnock();
		Serial.println("idendtifying as: ");
		Serial.println(id);
		if (id == OVERRIDE_ID) 
		{
			drawOverrideMode();
			initWifi();
			currentState = eLOOP_STATE_PENDING;
		}
		else if (id != NO_ID)
		{
			currentId = id;
			drawIdentiying(id);
			/* wifi wake up */
			initWifi();
			
			char messagePayload[MESSAGE_MAX_LEN];
			generateSequencePayload(id, messageCount, messagePayload);
			sendMessage(iotHubClientHandle, messagePayload);
			messageCount++;
			currentState = eLOOP_STATE_PENDING;
			Serial.println("state change to pending");
		}
		else 			
		{
			drawWrongId();
		}
	}
}

void challengeRoutine()
{
	static uint8_t tryNumber = 0;
	static bool showChallengeNeeded = true;
	if (showChallengeNeeded)
	{
		switch (tryNumber) 
		{
		case eCHALLENGE_MATRIX:
			char matrixChallengeCharArray[30];
			matrixChallengeStr.toCharArray(matrixChallengeCharArray, (unsigned int)matrixChallengeStr.length());
			drawMatrixChallenge(matrixChallengeCharArray, 10, 10, 140);
			currentChallengeSequence = matrixChallengeSequence;
			Serial.println("matrix display initiated");
			break;
		case eCHALLENGE_MENU:
			char menuChallengeCharArray[21 * 5];
			menuChallengeStr.toCharArray(menuChallengeCharArray, menuChallengeStr.length());
			drawMenuChallenge(menuChallengeCharArray, 10, 10, 140);
			currentChallengeSequence = menuChallengeSequence;
			break;
		case eCHALLENGE_KEYWORD:
			char passwordChallengeCharArray[50];
			//passwordChallengeStr.replace("0", "");
			passwordChallengeStr.toCharArray(passwordChallengeCharArray, passwordChallengeStr.length());
			drawPasswordChallenge(passwordChallengeCharArray, 10, 10, 140);
			currentChallengeSequence = passwordChallengeSequence;
			break;
		}
		showChallengeNeeded = false;
	}
	if (tryNumber == MAX_TRIALS)
	{
		alertRoutine();
		resetFunc();
	}
	knockSensorValue = analogRead(knockSensor);
	//drawAnalogRead(knockSensorValue);
	
	if (knockSensorValue >= threshold) {
		showChallengeNeeded = true;
		tryNumber++;
		bool result = checkChallenge();
		if (result)
		{
			openLock();
		}
		Serial.println("checkChallenge result: ");
		Serial.println(result);
	}
}

void wifiTurnOff() 
{
	WiFi.disconnect();
	delay(1);
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(3);
}

void Blink()
{
	/*
	digitalWrite(greenLED, LOW);
	delay(10);
	digitalWrite(greenLED, HIGH);
	delay(100);
	digitalWrite(greenLED, LOW);
	*/
}

void alertRoutine()
{
	digitalWrite(waterSplash, HIGH);
	delay(500);
	digitalWrite(waterSplash, LOW);
	digitalWrite(redAlert, HIGH);
	delay(10000);
	digitalWrite(redAlert, LOW);
	resetFunc();
}

int openLock()
{
	Serial.println("success");
	drawLockOpen();
	//displayText("success!", 1);
	Servo myServo1;
	myServo.detach();
	myServo1.attach(lockMotor);
	//myServo.write(0);
	myServo1.write(70);
	delay(2000);
	currentState = eLOOP_STATE_OPEN;

	return 200;
}
