#include <ESP8266WiFi.h>
#include "config_otk.h"

uint8_t* currentChallengeSequence;
uint8_t matrixChallengeSequence[3];
String matrixChallengeStr;
uint8_t menuChallengeSequence[3];
String menuChallengeStr;
uint8_t passwordChallengeSequence[3];
String passwordChallengeStr;

//config_eChallenge  global_challengeNo;

uint8_t identifyKnock(){
	uint16_t knockReadings[MAX_SEQUENCE_LENGTH];   // When someone knocks this array fills with delays between knocks.
	uint8_t knockReadingsLength = 0;
	Serial.println("identifying...");
	listenToKnock(knockReadings, &knockReadingsLength);
	
	if (0 < knockReadingsLength && knockReadingsLength <= MAX_ID_SEQUENCE_LENGTH && knockReadings[0] < MAX_ID)
	{
		// for now id can only be a single knock
		return knockReadings[0] + 1;
	}
	else if (knockReadings[0] == 2 && knockReadings[1] == 1 && knockReadings[2] == 2)
	{
		return OVERRIDE_ID;
	}
	else{
		return NO_ID;
	}
}

bool checkChallenge(){
	uint16_t knockReadings[MAX_SEQUENCE_LENGTH];
	uint8_t knockReadingsLength = 0;
	bool res = true;
	Serial.println("challenge knocking");
	listenToKnock(knockReadings, &knockReadingsLength);
	//char knock_str[MAX_SEQUENCE_STR]; //todo: convert sequence to str
	//readingsToStr(knockReadings, knockReadingsLength, knock_str, MAX_SEQUENCE_STR);
	Serial.println("/* checking sequence */");

	// handle 1,1,1 knock sequence (special ambiguous case)
	if (knockReadings[1] == 0 && knockReadingsLength == 3)
	{
		knockReadings[0] = 1;
		knockReadings[1] = 1;
		knockReadings[2] = 1;
	}

	int errorCount = 0;
	for (uint8_t i = 0; i < 3; i++)
	{
		Serial.print("expected: ");
		//tft.print("expected: ");
		Serial.println(currentChallengeSequence[i]);
		//tft.println(currentChallengeSequence[i]);
		Serial.print("actual: ");
		//tft.print("actual: ");
		Serial.println(knockReadings[i]);
		//tft.println(knockReadings[i]);
		errorCount += abs(knockReadings[i] - currentChallengeSequence[i]);
	}
	if (errorCount > errorThreshold) 
	{
		res = false;
	}

	return res;

}

// Records the timing of knocks.
void listenToKnock(uint16_t* out_sequence, uint8_t* out_sequence_length) {
	Serial.println("knock starting");
	//tft.setCursor(0, 115);
	//tft.print("knock ");
	/* if (safetyCount - millis() > 50) {
		//Serial.println(knockSensorValue);
		safetyCount = millis();
		//delay(3);
		yield();
		safetyCount = 0;
	} */
	int now = millis();
	int startTime = millis();                 // Reference for when this knock started.

	int i = 0;
	// First lets reset the listening array.
	for (i = 0; i < maximumKnocks; i++) {
		out_sequence[i] = 0;
	}

	int currentKnockNumber = 0;               // Incrementer for the array.
	//Blink();
	int maxKnockInterval = 0;               // We use this later to normalize the times.
	int minKnockInterval = knockComplete;
	bool singleKnock = false;

	drawKnockingIndicator(true);
	delay(knockFadeTime);                                 // wait for this peak to fade before we listen to the next one.
	drawKnockingIndicator(false);
	do {
		//listen for the next knock or wait for it to timeout. 
		knockSensorValue = analogRead(knockSensor);
		//drawAnalogRead(knockSensorValue);

		if (knockSensorValue >= threshold) {                   //got another knock...
															   //record the delay time.
			Serial.println("knock.");
			//tft.print("knock ");
			//Serial.println(knockSensorValue);
			now = millis();
			//Blink();
			out_sequence[currentKnockNumber] = now - startTime;
			int test = now - startTime;
			Serial.println(test);
			if (out_sequence[currentKnockNumber] > maxKnockInterval) 
			{
				maxKnockInterval = out_sequence[currentKnockNumber];
			}
			if (out_sequence[currentKnockNumber] < minKnockInterval)
			{
				minKnockInterval = out_sequence[currentKnockNumber];
			}
			currentKnockNumber++;                             //increment the counter
			startTime = now;
			
			drawKnockingIndicator(true);
			delay(knockFadeTime);                              // again, a little delay to let the knock decay.
			drawKnockingIndicator(false);

		}
		now = millis();
		yield();
		//did we timeout or run out of knocks?
	} while ((now - startTime < knockComplete) && (currentKnockNumber < maximumKnocks));

	/* check if single knock or sequence */
	if (map(minKnockInterval, 0, maxKnockInterval, 0, 100) > SEQUENCE_INTERVAL_THRESHOLD || currentKnockNumber == 0)
	{
		singleKnock = true;
		out_sequence[0] = currentKnockNumber;
		*out_sequence_length = 1;
		Serial.println("single knock digit");
		return;
	}

	/*  Convert time intervals to digit sequence */
	int totaltimeDifferences = 0;
	int timeDiff = 0;
	uint8_t k = 0;
	uint8_t currentKnockCount = 1;
	for (i = 0; i < maximumKnocks; i++) { 
		out_sequence[i] = map(out_sequence[i], 0, maxKnockInterval, 0, 100);
		if(0 == out_sequence[i]){
			//sequence is finished
			out_sequence[k++] = currentKnockCount;
			Serial.print(currentKnockCount);
			goto end_of_sequence;
		} else if (out_sequence[i] > SEQUENCE_INTERVAL_THRESHOLD)
		{
			out_sequence[k++] = currentKnockCount;
			Serial.print(currentKnockCount);
			Serial.print(", ");
			currentKnockCount = 1;
		} else 
		{
			currentKnockCount++;
		}
	}

	end_of_sequence:
	*out_sequence_length = k;
	Serial.println(k);
	return;
}

void readingsToStr(uint8_t* knockReadings, uint8_t readingSize, char* knockStr, uint8_t knockStrSize) {
	for (uint8_t i = 0; i <= readingSize; ++i)
	{
		knockStr[2 * i] = knockReadings[i] + '0';
		knockStr[2 * i + 1] = ',';
	}
	/* last number without a comma */
	knockStr[2 * (readingSize - 1)] = knockReadings[readingSize - 1] + '0';
	knockStr[2 * (readingSize - 1) + 1] = '\0';
	Serial.println("knock Reading str: ");
	Serial.println(knockStr);
}
// Runs the motor (or whatever) to unlock the door.
void triggerDoorUnlock() {
	//Serial.println("Door unlocked!");
	//int i = 0;

	//// turn the motor on for a bit.
	//servo.write(90);
	//digitalWrite(greenLED, HIGH);            // And the green LED too.
	//delay(2000);

	//delay(lockTurnTime);                    // Wait a bit.

	//digitalWrite(lockMotor, LOW);            // Turn the motor off.

	//										 // Blink the green LED a few times for more visual feedback.
	//for (i = 0; i < 5; i++) {
	//	digitalWrite(greenLED, LOW);
	//	delay(100);
	//	digitalWrite(greenLED, HIGH);
	//	delay(100);
	//}
}