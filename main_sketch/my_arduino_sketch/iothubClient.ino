#include "config_otk.h"
#include <ArduinoJson.h>


static WiFiClientSecure sslClient; // for ESP8266
const char *onSuccess = "\"Successfully invoke device method\"";
const char *notFound = "\"No method found\"";

//extern char const * global_challengeSequence;
//extern config_eChallenge  global_challengeNo;
extern uint8_t matrixChallengeSequence[3];
extern String matrixChallengeStr;
extern uint8_t menuChallengeSequence[3];
extern String menuChallengeStr;
extern uint8_t passwordChallengeSequence[3];
extern String passwordChallengeStr;

/*
 * The new version of AzureIoTHub library change the AzureIoTHubClient signature.
 * As a temporary solution, we will test the definition of AzureIoTHubVersion, which is only defined
 *    in the new AzureIoTHub library version. Once we totally deprecate the last version, we can take
 *    the #ifdef out.
 * Break changes in version 1.0.34: AzureIoTHub library removed AzureIoTClient class.
 * So we remove the code below to avoid compile error.
 */

/*
 * #ifdef AzureIoTHubVersion
 * static AzureIoTHubClient iotHubClient;
 * void initIoThubClient()
 * {
 *     iotHubClient.begin(sslClient);
 * }
 * #else
 * static AzureIoTHubClient iotHubClient(sslClient);
 * void initIoThubClient()
 * {
 *     iotHubClient.begin();
 * }
 * #endif
 */
 
static void sendCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    if (IOTHUB_CLIENT_CONFIRMATION_OK == result)
    {
        Serial.println("Message sent to Azure IoT Hub.");
        //blinkLED();
    }
    else
    {
        Serial.println("Failed to send message to Azure IoT Hub.");
    }
    //messagePending = false;
}

static void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, char *buffer)
{
	Serial.printf("Trying to send message: %s.\r\n", buffer);
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char *)buffer, strlen(buffer));
    if (messageHandle == NULL)
    {
        Serial.println("Unable to create a new IoTHubMessage.");
    }
    else
    {
        MAP_HANDLE properties = IoTHubMessage_Properties(messageHandle);
        //Map_Add(properties, "temperatureAlert", temperatureAlert ? "true" : "false");
        Serial.printf("Sending message: %s.\r\n", buffer);
        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, sendCallback, NULL) != IOTHUB_CLIENT_OK)
        {
            Serial.println("Failed to hand over the message to IoTHubClient.");
        }
        else
        {
            //messagePending = true;
            Serial.println("IoTHubClient accepted the message for delivery.");
        }

        IoTHubMessage_Destroy(messageHandle);
    }
}

IOTHUBMESSAGE_DISPOSITION_RESULT receiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void *userContextCallback)
{
    IOTHUBMESSAGE_DISPOSITION_RESULT result;
    const unsigned char *buffer;
    size_t size;
    if (IoTHubMessage_GetByteArray(message, &buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        Serial.println("Unable to IoTHubMessage_GetByteArray.");
        result = IOTHUBMESSAGE_REJECTED;
    }
    else
    {
        /*buffer is not zero terminated*/
        char temp[MESSAGE_MAX_LEN];
        strncpy(temp, (const char *)buffer, size);
        temp[size] = '\0';

        Serial.printf("Receive C2D message: %s.\r\n", temp);
    }
    return IOTHUBMESSAGE_ACCEPTED;
}

int deviceMethodCallback(const char *methodName, const unsigned char *payload, size_t size, unsigned char **response, size_t *response_size, void *userContextCallback)
{
    Serial.printf("Try to invoke method %s.\r\n", methodName);
    const char *responseMessage = onSuccess;
    int result = 200;

    if (strcmp(methodName, "startChallenge") == 0)
    {
		result = startChallenge(payload, size);
    }
	else if (strcmp(methodName, "openLock") == 0) 		
	{
		result = remoteOpenLock(payload, size);
	}
    else
    {
        Serial.printf("No method %s found.\r\n", methodName);
        responseMessage = notFound;
        result = 404;
    }

    *response_size = strlen(responseMessage);
    *response = (unsigned char *)malloc(*response_size);
    strncpy((char *)(*response), responseMessage, *response_size);

    return result;
}

void twinCallback(
    DEVICE_TWIN_UPDATE_STATE updateState,
    const unsigned char *payLoad,
    size_t size,
    void *userContextCallback)
{
	/*
    char *temp = (char *)malloc(size + 1);
    for (int i = 0; i < size; i++)
    {
        temp[i] = (char)(payLoad[i]);
    }
    temp[size] = '\0';
    parseTwinMessage(temp);
    free(temp);
	*/
}

void generateSequencePayload(uint8_t userId, int messageId, char *payload)
{
    StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["deviceId"] = DEVICE_ID;
    root["messageId"] = messageId;
    root["azureRoute"] = "generateChallenge";
    root["userId"] = userId;
    
    root.printTo(payload, MESSAGE_MAX_LEN);
}

int remoteOpenLock(const unsigned char *payload, size_t size) 	
{
	int result = 200;

	char message[MESSAGE_MAX_LEN];
	strncpy(message, (const char *)payload, size);
	message[size] = '\0';
	StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
	JsonObject &root = jsonBuffer.parseObject(message);
	if (!root.success()) {
		Serial.printf("Parse %s failed.\r\n", message);
		result = 404;
		return result;
	}
	if (root["id"].success()) 
	{
		uint8_t id = root["id"];
		currentId = id;
		Serial.println("open ordered by id: ");
		Serial.println(id);
		result = openLock();
	}
	return result;
}

int startChallenge(const unsigned char *payload, size_t size)
{
	int result = 200;

	char message[MESSAGE_MAX_LEN];
	strncpy(message, (const char *)payload, size);
	message[size] = '\0';
	StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
	JsonObject &root = jsonBuffer.parseObject(message);
	if (!root.success())
	{
		Serial.printf("Parse %s failed.\r\n", message);
		result = 404;
		return result;
	}

	if (root["matrixChallenge"].success())
	{
		JsonObject &matrixChallenge = root["matrixChallenge"];
		//JsonArray &challengeMatrices = matrixChallenge.createNestedArray("challengeMatrices");

		//parse string for display
		for (int i = 0; i < 3; i++)
		{
			String matrixString = matrixChallenge["challengeMatrices"][i];
			matrixChallengeStr += matrixString;
		}
		Serial.println("matrixStr: ");
		Serial.println(matrixChallengeStr);

		//parse sequence for knocking
		Serial.println("matrixSequence: ");
		for (int i = 0; i < 3; i++)
		{
			//int matrix = matrixChallenge["challengeMatrices"][i];
			matrixChallengeSequence[i] = matrixChallenge["challengeSequence"][i];
			Serial.print(matrixChallengeSequence[i]);
			Serial.print(", ");
		}
		Serial.println("");
	}

	if (root["menuChallenge"].success())
	{
		JsonObject &menuChallenge = root["menuChallenge"];
		//JsonArray &challengeMatrices = matrixChallenge.createNestedArray("challengeMatrices");

		//parse string for display
		for (int i = 0; i < 5; i++)
		{
			String menuString = menuChallenge["challengeMenu"][i];
			menuChallengeStr += menuString;
			menuChallengeStr += '\n';
		}
		Serial.println("menuStr: ");
		Serial.println(menuChallengeStr);

		//parse sequence for knocking
		Serial.println("menuSequence: ");
		for (int i = 0; i < 3; i++)
		{
			menuChallengeSequence[i] = menuChallenge["challengeSequence"][i];
			Serial.print(menuChallengeSequence[i]);
			Serial.print(", ");
		}
		Serial.println("");
	}

	if (root["passwordChallenge"].success())
	{
		JsonObject &passwordChallenge = root["passwordChallenge"];
		//JsonArray &challengeMatrices = matrixChallenge.createNestedArray("challengeMatrices");

		//parse string for display
		for (int i = 0; i < 3; i++)
		{
			String passwordString = passwordChallenge["challengePasswords"][i];
			passwordChallengeStr += passwordString;
			passwordChallengeStr += '\n';
		}
		Serial.println("passwordStr: ");
		Serial.println(passwordChallengeStr);

		//parse sequence for knocking
		Serial.println("passwordSequence: ");
		for (int i = 0; i < 3; i++)
		{
			passwordChallengeSequence[i] = passwordChallenge["challengeSequence"][i];
			Serial.print(passwordChallengeSequence[i]);
			Serial.print(", ");
		}
		Serial.println("");
	}


	// startChallenge(); //todo: handle challenge display on screen
	currentState = eLOOP_STATE_CHALLENGE_READY;
	Serial.println("state change to challenge ready");
	//digitalWrite(greenLED, HIGH);
}