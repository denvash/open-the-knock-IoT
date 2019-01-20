/*
An example analogue clock using a TFT LCD screen to show the time
use of some of the drawing commands with the ST7735 library.

For a more accurate clock, it would be better to use the RTClib library.
But this is just a demo.

Uses compile time to set the time so a reset will start with the compile time again

Gilchrist 6/2/2014 1.0
Updated by Bodmer
*/
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include "Setup2_ST7735.h"
#include <JPEGDecoder.h>
#include "JPEG_logo.h"
#include <FS.h>
#include "Free_Fonts.h"

#define minimum(a,b)     (((a) < (b)) ? (a) : (b))
#define SWAP_RED_BLUE(u) ((u << 11) | ((uint16)u >> 11) | (u & 0x07E0))
#define COLOR_CORRECT(u) ~(SWAP_RED_BLUE(u))

#define MY_BLACK		COLOR_CORRECT(TFT_BLACK)       
#define MY_NAVY			COLOR_CORRECT(TFT_NAVY)    
#define MY_DARKGREEN	COLOR_CORRECT(TFT_DARKGREEN)  
#define MY_DARKCYAN		COLOR_CORRECT(TFT_DARKCYAN)  
#define MY_MAROON		COLOR_CORRECT(TFT_MAROON)   
#define MY_PURPLE		COLOR_CORRECT(TFT_PURPLE)     
#define MY_OLIVE		COLOR_CORRECT(TFT_OLIVE)     
#define MY_LIGHTGREY	COLOR_CORRECT(TFT_LIGHTGREY)
#define MY_DARKGREY		COLOR_CORRECT(TFT_DARKGREY)  
#define MY_BLUE			COLOR_CORRECT(TFT_BLUE)   
#define MY_GREEN		COLOR_CORRECT(TFT_GREEN)       
#define MY_CYAN			COLOR_CORRECT(TFT_CYAN)      
#define MY_RED			COLOR_CORRECT(TFT_RED)       
#define MY_MAGENTA		COLOR_CORRECT(TFT_MAGENTA)     
#define MY_YELLOW		COLOR_CORRECT(TFT_YELLOW)    
#define MY_WHITE		COLOR_CORRECT(TFT_WHITE)     
#define MY_ORANGE		COLOR_CORRECT(TFT_ORANGE)      
#define MY_GREENYELLOW	COLOR_CORRECT(TFT_GREENYELLOW) 
#define MY_PINK			COLOR_CORRECT(TFT_PINK)

#define INVERSE_BLUE 0xFFE0;

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

void initScreen(void) {
	tft.init();
	tft.setRotation(1);

	/* init file system */
	if (!SPIFFS.begin()) {
		Serial.println("SPIFFS initialisation failed!");
		while (1) yield(); // Stay here twiddling thumbs waiting
	}
	Serial.println("\r\n FileSystem Initialisation done.");

	String fileName = "Final-Frontier-28";
	//tft.loadFont(fileName);
	tft.fillScreen(MY_WHITE);
	tft.setTextColor(MY_BLACK, MY_WHITE);  // Adding a black background colour erases previous text automatically
	tft.setTextSize(1);
	tft.setTextWrap(true, true);

		//drawMenuChallenge("Heiniken....5$\nCastille...3$\nCocaCola..2$", 10, 10, 140);
		//delay(10000);
		//drawMatrixChallenge("101010101101010101101010101", 10, 10, 140);
		//delay(10000);

}

void drawWelcome()
{
	tft.fillScreen(MY_WHITE);
	//tft.drawCentreString("Open The Knock", (80), 64, 4);
	drawArrayJpeg(Tiger, sizeof(Tiger), 20, 27); // Draw a jpeg image stored in memory at x,y
}

void drawOverrideMode() 
{
	tft.fillScreen(MY_WHITE);
	tft.drawCentreString("Override mode", 80, 50, 2);
	// must first initialize the String to concantenate
}

void drawIdentiying(int id) 
{
	tft.fillScreen(MY_WHITE);

	String myStr = "Contacting Cloud...";
	tft.drawCentreString(myStr, 80, 40, 2);
	// must first initialize the String to concantenate
	String idStr = "Identifying as ";
	myStr = idStr + id;
	tft.drawCentreString(myStr, 80, 60, 2);
}

void drawWrongId() 
{
	tft.fillScreen(MY_WHITE);
	tft.drawCentreString("Wrong ID", 80, 40, 2);
	tft.drawCentreString("Knock again!", 80, 60, 2);
	delay(5000);
	drawWelcome();
}

void drawAnalogRead(int analogRead){
	static int lastCall = millis();
	if (millis() - lastCall > 10) 		
	{
		lastCall = millis();
		tft.setCursor(0, 0);
		tft.print(analogRead);
	}
}

void drawConnectingFailed() 
{
	tft.fillScreen(MY_WHITE);
	tft.drawCentreString("WiFi Acquiring failed", 80, 40, 2);
	tft.drawCentreString("reseting...", 80, 60, 2);
}

void drawLocking()
{
	tft.fillScreen(MY_WHITE);
	tft.drawCentreString("Locking Door...", 80, 55, 2);
}

void drawCloudResponse(String response) 	
{
	tft.fillScreen(MY_WHITE);
	tft.drawCentreString(response, 80, 40, 2);
	tft.drawCentreString("reseting...", 80, 60, 2);
}

void drawLockOpen() {
	tft.fillScreen(MY_WHITE);
	tft.drawCentreString("Lock open", 80, 55, 2);
}


void drawMatrix(char* matrixStr, int32_t x0, int32_t y0, int32_t width) {
	const int32_t matrixSize = 3;
	const int32_t boxSize = width / matrixSize;
	int32_t xCursor = x0;
	int32_t yCursor = y0;

	tft.fillRect(x0, y0, width, width, MY_WHITE);
	tft.drawRect(x0, y0, width, width, MY_BLACK);
	for (uint8_t y = 0; y < matrixSize; y++)
	{
		for (uint8_t x = 0; x < matrixSize; x++)
		{
			if (matrixStr[y * matrixSize + x] == '1')
			{
				tft.fillRect(xCursor, yCursor, boxSize, boxSize, MY_BLACK);
			}
			xCursor += boxSize;
			tft.drawFastVLine(xCursor, y0, width, MY_BLACK);
		}
		xCursor = x0;
		yCursor += boxSize;
		tft.drawFastHLine(x0, yCursor, width, MY_BLACK);
	}
}

void drawKnockingIndicator(bool on) 
{
	uint32_t color = on ? MY_BLACK : MY_WHITE;
	tft.fillCircle(tft.width() - 20, tft.height() - 20, 8, color);
}

void drawMatrixChallenge(char* matrixStr, int32_t x0, int32_t y0, int32_t width)
{
	tft.fillScreen(MY_WHITE);

	if (!matrixStr)
	{
		tft.drawCentreString("bad ptr", 80, 80, 4);
		return;
	}

	tft.drawCentreString("Matrix Challenge", (width / 2) + x0, y0, 2);
	y0 += 30;

	const uint8_t challengeLength = 3;
	const int32_t matrixSpace = 5;
	int32_t matrixWidth = (width - ( (challengeLength - 1) * matrixSpace) ) / 3 ;
	
	for (uint8_t i = 0; i < challengeLength; i++)
	{
		drawMatrix(matrixStr, x0, y0, matrixWidth);
		matrixStr += 9;
		x0 += matrixSpace + matrixWidth;
	}
}

void displayText(char* myText, int size)
{
	tft.setTextSize(size);				// We are using a text size multiplier of 1
	tft.setTextColor(MY_BLACK);			// Set text colour to black, no background (so transparent)
	tft.setCursor(10, 80, 2);			// Set cursor to x = 76, y = 150 and use font 4
	tft.println(myText);				// As we use println, the cursor moves to the next line

	//tft.drawCentreString(myText, 80, 80, size);
}

void drawMenuChallenge(char* menuStr, int32_t x0, int32_t y0, int32_t width)
{
	tft.fillScreen(MY_WHITE);
	uint16_t currentLineIndex = 0;
	char tmp[200];

	String(menuStr).toCharArray(tmp, 200, 0);
	if (!menuStr)
	{
		tft.drawCentreString("bad ptr", 80, 80, 4);
		return;
	}

	tft.drawCentreString("Menu Challenge", (width / 2) + x0, y0, 2);
	y0 += 30;
	for (uint16_t i = 0; tmp[i] != '\0'; i++)
	{
		if (menuStr[i] == '\n')
		{
			tmp[i] = '\0';
			tft.drawCentreString(&tmp[currentLineIndex], (width / 2) + x0, y0, 1);
			currentLineIndex = i + 1;
			y0 += 10;
		}
	}
	tft.drawCentreString(&tmp[currentLineIndex], (width / 2) + x0, y0, 1);

	//tft.print(menuStr);
}

void drawPasswordChallenge(char* passwordStr, int32_t x0, int32_t y0, int32_t width)
{
	tft.fillScreen(MY_WHITE);
	uint16_t currentLineIndex = 0;
	char tmp[31];
	String(passwordStr).toCharArray(tmp, 100, 0);
	if (!passwordStr)
	{
		tft.drawCentreString("bad ptr", 80, 80, 4);
		return;
	}

	tft.drawCentreString("Password Challenge", (width / 2) + x0, y0, 2);
	y0 += 30;
	for (uint16_t i = 0; tmp[i] != '\0'; i++)
	{
		if (passwordStr[i] == '\n')
		{
			tmp[i] = '\0';
			tft.drawCentreString(&tmp[currentLineIndex], (width / 2) + x0, y0, 2);
			currentLineIndex = i + 1;
			y0 += 20;
		}
	}
	//tft.setFreeFont(FF2);
	tft.drawCentreString(&tmp[currentLineIndex], (width / 2) + x0, y0, 2);

	//tft.print(menuStr);
}

void drawArrayJpeg(const uint8_t arrayname[], uint32_t array_size, int xpos, int ypos) {

	int x = xpos;
	int y = ypos;

	JpegDec.scanType = PJPG_GRAYSCALE;
	JpegDec.decodeArray(arrayname, array_size);

	//jpegInfo(); // Print information from the JPEG file (could comment this line out)

	renderJPEG(x, y);

	Serial.println("#########################");
}

void renderJPEG(int xpos, int ypos) {

	// retrieve infomration about the image
	uint16_t *pImg;
	uint16_t mcu_w = JpegDec.MCUWidth;
	uint16_t mcu_h = JpegDec.MCUHeight;
	uint32_t max_x = JpegDec.width;
	uint32_t max_y = JpegDec.height;

	// Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
	// Typically these MCUs are 16x16 pixel blocks
	// Determine the width and height of the right and bottom edge image blocks
	uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
	uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

	// save the current image block size
	uint32_t win_w = mcu_w;
	uint32_t win_h = mcu_h;

	// record the current time so we can measure how long it takes to draw an image
	uint32_t drawTime = millis();

	// save the coordinate of the right and bottom edges to assist image cropping
	// to the screen size
	max_x += xpos;
	max_y += ypos;

	// read each MCU block until there are no more
	while (JpegDec.readSwappedBytes()) {
		// save a pointer to the image block
		pImg = JpegDec.pImage;

		// calculate where the image block should be drawn on the screen
		int mcu_x = JpegDec.MCUx * mcu_w + xpos;  // Calculate coordinates of top left corner of current MCU
		int mcu_y = JpegDec.MCUy * mcu_h + ypos;

		// check if the image block size needs to be changed for the right edge
		if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
		else win_w = min_w;

		// check if the image block size needs to be changed for the bottom edge
		if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
		else win_h = min_h;

		// copy pixels into a contiguous block
		if (win_w != mcu_w)
		{
			uint16_t *cImg;
			int p = 0;
			cImg = pImg + win_w;
			for (int h = 1; h < win_h; h++)
			{
				p += mcu_w;
				for (int w = 0; w < win_w; w++)
				{
					*cImg = *(pImg + w + p);
					cImg++;
				}
			}
		}

		// draw image MCU block only if it will fit on the screen
		if ((mcu_x + win_w) <= tft.width() && (mcu_y + win_h) <= tft.height())
		{
			//picColorCorrect(pImg, win_w, win_h);
			tft.pushRect(mcu_x, mcu_y, win_w, win_h, pImg);
		}
		else if ((mcu_y + win_h) >= tft.height()) JpegDec.abort(); // Image has run off bottom of screen so abort decoding
	}

	// calculate how long it took to draw the image
	drawTime = millis() - drawTime;

	// print the results to the serial port
	Serial.print(F("Total render time was    : ")); Serial.print(drawTime); Serial.println(F(" ms"));
	Serial.println(F(""));
}

void picColorCorrect(uint16_t* orig, uint32_t win_w, uint32_t win_h) {
	uint8_t* orig8 = (uint8_t*) orig;
	uint32_t ptr = 0;
	int p = 0;
	for (int h = 0; h < win_h; h++)
	{
		for (int w = 0; w < win_w; w++)
		{
			orig[ptr] = ~orig[ptr];
			//uint8_t tmp = orig8[ptr];
			//orig8[ptr] = reverse(orig8[ptr+1]);
			//orig8[ptr+1] = reverse(tmp);
			ptr ++;
		}
	}
}

unsigned char reverse(unsigned char b) {
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}