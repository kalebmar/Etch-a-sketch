#include "display_app.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "display_driver.h"
#include "hardware.h"

/************************************************************
 Variables
 ************************************************************/

uint8_t prevYpixel = 0;
uint8_t prevXpixel = 0;

/************************************************************
 Functions
 ************************************************************/

// Initialises the display: resets, configures, turns it on, clears it, and sets initial brightness
void displayInit() {

	enableBuffer();

	selectDisplaySide(LEFT_AND_RIGHT_SIDE);
	waitForTiming();

	resetDisplay();

	waitForTiming();

	setDisplay();

	// Reset time is at least 1 us
	for (uint8_t i = 0; i < 4; i++)
		waitForTiming();

	displayON();
	waitForTiming();

	deselectDisplaySide(LEFT_AND_RIGHT_SIDE);
	waitForTiming();

	//Z address is set to 0 by default after RESET
	clearDisplay();
	setDisplayPWMPulse(1000);
}

// Clears the display and pseudoRAM by writing zeros to all pixels
void clearDisplay() {
	memset(pseudoRAM, 0, 1024); // Clear pseudoRAM

	for (uint8_t i = 0; i < 8; i++) {
		setAddressX(i, LEFT_AND_RIGHT_SIDE); // In one page there is 8px

		setAddressY(0, LEFT_AND_RIGHT_SIDE); // Display use y for horizontal x for vertical

		for (uint8_t j = 0; j < 64; j++) {
			writeDisplay(0, LEFT_SIDE);
			writeDisplay(0, RIGHT_SIDE);
		}
	}
}

// Floods entire display with pixels
void floodDisplay() {
	for (uint8_t i = 0; i < 8; i++) {
		setAddressX(i, LEFT_AND_RIGHT_SIDE); // In one page there is 8px
		setAddressY(0, LEFT_AND_RIGHT_SIDE); // Display use y for horizontal x for vertical

		for (uint8_t j = 0; j < 64; j++) {
			writeDisplay(0xFF, LEFT_AND_RIGHT_SIDE);
		}
	}
}

// Sends display buffer content over UART
// NOT WORKING CORRECTLY
void sendDisplayData() {
	char msg[10];
	displayOFF();
	for (int i = 0; i < 1024; i++) {
		sprintf(msg, "%hu\r\n", pseudoRAM[i]);
		HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);
	}
	displayON();
}

// Receives display buffer via UART and shows image
// NOT WORKING CORRECTLY
void loadDisplayData() {
	displayOFF();
	HAL_UART_Receive(&huart2, pseudoRAM, sizeof(pseudoRAM), HAL_MAX_DELAY);
	displayON();
	clearDisplay();
	pictureToDisplay(pseudoRAM);
}

// Draw single pixel to display based on raw X and Y input values
void drawToDisplay(uint16_t rawValueX, uint16_t rawValueY) {
	uint8_t xPixel = valueToPixelCol(rawValueX);
	uint8_t yPixel = valueToPixelRow(rawValueY);
	uint16_t index = xPixel + (yPixel / 8) * 128;

	pseudoRAM[index] = pseudoRAM[index] | (1 << (yPixel % 8));
	if (xPixel < 64) { // 0-63 left side CS1
		setAddressX(yPixel / 8, LEFT_SIDE); // In one page there is 8px
		setAddressY(xPixel, LEFT_SIDE); // Display use y for horizontal x for vertical
		writeDisplay(pseudoRAM[index], LEFT_SIDE);

	}
	else { // 64-127 right side CS2
		setAddressX(yPixel / 8, RIGHT_SIDE); // In one page there is 8px
		setAddressY(xPixel - 64, RIGHT_SIDE); // Display use y for horizontal x for vertical
		writeDisplay(pseudoRAM[index], RIGHT_SIDE);
	}
}

// Turn off drawing, but update cursor location on screen
void cursorOff(uint16_t rawValueX, uint16_t rawValueY) {
	uint8_t data;
	uint8_t xPixel = valueToPixelCol(rawValueX);
	uint8_t yPixel = valueToPixelRow(rawValueY);
	uint16_t index = xPixel + (yPixel / 8) * 128;

	if (prevXpixel < 64) { // 0-63 left side CS1
		setAddressX(prevYpixel / 8, LEFT_SIDE); // In one page there is 8px
		setAddressY(prevXpixel, LEFT_SIDE); // Display use y for horizontal x for vertical
		writeDisplay(pseudoRAM[prevXpixel + (prevYpixel / 8) * 128], LEFT_SIDE);

	}
	else { // 64-127 right side CS2
		setAddressX(prevYpixel / 8, RIGHT_SIDE); // In one page there is 8px
		setAddressY(prevXpixel - 64, RIGHT_SIDE); // Display use y for horizontal x for vertical
		writeDisplay(pseudoRAM[prevXpixel + (prevYpixel / 8) * 128],
				RIGHT_SIDE);
	}

	data = pseudoRAM[index] | (1 << (yPixel % 8));
	if (xPixel < 64) { // 0-63 left side CS1
		setAddressX(yPixel / 8, LEFT_SIDE); // In one page there is 8px
		setAddressY(xPixel, LEFT_SIDE); // Display use y for horizontal x for vertical
		writeDisplay(data, LEFT_SIDE);

	}
	else { // 64-127 right side CS2
		setAddressX(yPixel / 8, RIGHT_SIDE); // In one page there is 8px
		setAddressY(xPixel - 64, RIGHT_SIDE); // Display use y for horizontal x for vertical
		writeDisplay(data, RIGHT_SIDE);
	}

	prevXpixel = xPixel;
	prevYpixel = yPixel;

}

// Write a full picture to the display from a byte array
// The array contains 8 pages (rows), each with 128 bytes (columns)
// Each byte represents 8 vertical pixels in a column within a page
void pictureToDisplay(const uint8_t array[]) {
	uint8_t xPixel;
	uint8_t yPixel;
	uint8_t data;

	for (yPixel = 0; yPixel < 8; yPixel++) {
		for (xPixel = 0; xPixel < 128; xPixel++) {
			if (xPixel < 64) { // 0-63 left side CS1
				setAddressX(yPixel, LEFT_SIDE); // In one page there is 8px
				data = array[xPixel + 128 * yPixel];
				writeDisplay(data, LEFT_SIDE);
			}
			else { // 64-127 right side CS2
				setAddressX(yPixel, RIGHT_SIDE); // In one page there is 8px
				data = array[xPixel + 128 * yPixel];
				writeDisplay(data, RIGHT_SIDE);
			}
		}
	}
}

// Compare new image to previous image
// Only update the display where they are different
// The arrays contain 8 pages (rows), each with 128 bytes (columns)
// Each byte represents 8 vertical pixels in a column within a page
void animationToDisplay(const uint8_t prevImage[], const uint8_t newImage[]) {
	uint8_t xPixel;
	uint8_t yPixel;
	uint8_t data;

	for (yPixel = 0; yPixel < 8; yPixel++) {
		for (xPixel = 0; xPixel < 128; xPixel++) {
			if(newImage[xPixel + 128 * yPixel] == prevImage[xPixel + 128 * yPixel])
				break;
			else if (xPixel < 64) { // 0-63 left side CS1
				setAddressX(yPixel, LEFT_SIDE); // In one page there is 8px
				data = newImage[xPixel + 128 * yPixel];
				writeDisplay(data, LEFT_SIDE);
				HAL_Delay(75);
			}
			else { // 64-127 right side CS2
				setAddressX(yPixel, RIGHT_SIDE); // In one page there is 8px
				data = newImage[xPixel + 128 * yPixel];
				writeDisplay(data, RIGHT_SIDE);
				HAL_Delay(75);
			}
		}
	}
}

// Read a byte from the display memory at given pixel coordinates (x, y)
uint8_t readFromDisplayTest(uint8_t xPixel, uint8_t yPixel) {
	uint8_t data;

	if (xPixel < 64) { // 0-63 left side CS1
		setAddressX(yPixel / 8, LEFT_SIDE); // In one page there is 8px
		setAddressY(xPixel, LEFT_SIDE); // Display use y for vertical x for horizontal
		data = readDisplay(LEFT_SIDE);
	}
	else { // 64-127 right side CS2
		xPixel = xPixel - 64;
		setAddressX(yPixel / 8, RIGHT_SIDE); // In one page there is 8px
		setAddressY(xPixel, RIGHT_SIDE); // Display use y for vertical x for horizontal
		data = readDisplay(RIGHT_SIDE);
	}
	return data;
}

// Write a byte to display memory at given pixel coordinates (x, y)
void drawToDisplayTest(uint8_t xPixel, uint8_t yPixel, uint8_t value) {

	if (xPixel < 64) { // 0-63 left side CS1
		setAddressX(yPixel / 8, LEFT_SIDE); // In one page there is 8px
		setAddressY(xPixel, LEFT_SIDE); // Display use y for vertical x for horizontal
		writeDisplay(value, LEFT_SIDE);

	}
	else { // 64-127 right side CS2
		xPixel = xPixel - 64;
		setAddressX(yPixel / 8, RIGHT_SIDE); // In one page there is 8px
		setAddressY(xPixel, RIGHT_SIDE); // Display use y for vertical x for horizontal
		writeDisplay(value, RIGHT_SIDE);
	}
}
