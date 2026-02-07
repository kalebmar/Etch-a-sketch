#include <display_driver.h>
#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "hardware.h"

/************************************************************
						Pins
 ************************************************************/

/*
PA5 DB0
PA6 DB1
PA7 DB2
PA8 DB4 !!!!!!!!!!
PA9 DB3 !!!!!!!!!!
PA10 DB5
PA11 DB6
PA12 DB7
*/

/************************************************************
						Display instructions
 ************************************************************/

// Instruction bits from MSB to LSB
// RS R/W DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0
// Renamed to
// DI RW DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0

// DISPLAY ON
// 0 0 0 0 1 1 1 1 1 D
// Display data when D H
// If D L data still remains in RAM
void displayON(){
	selectDisplaySide(LEFT_AND_RIGHT_SIDE);

	sendInstructionToDisplay();
	writeDisplayDataPin();

	waitForTiming();

	displayEnableSignal();
	writeDataBits(0b00111111); // Last bit H

	waitForTiming();

	displayDisableSignal();
	deselectDisplaySide(LEFT_AND_RIGHT_SIDE);
}

// DISPLAY OFF
// 0 0 0 0 1 1 1 1 1 D
// Doesn't display data when D L
// If D L data still remains in RAM
void displayOFF(){
	selectDisplaySide(LEFT_AND_RIGHT_SIDE);

	sendInstructionToDisplay();
	writeDisplayDataPin();

	waitForTiming();

	displayEnableSignal();
	writeDataBits(0b00111110); // Last bit L

	waitForTiming();

	displayDisableSignal();
	deselectDisplaySide(LEFT_AND_RIGHT_SIDE);
}

// SET ADDRESS (Y ADDRESS)
// 0 0 0 1 AC5 AC4 AC3 AC2 AC1 AC0
// Y address (AC0-AC5)
// Increased by 1 automatically by read or write operations
// This is the column
void setAddressY(uint8_t addressY, uint8_t sideSelect){
	if(addressY > 63)
		addressY = 0; // addressY can be 0-63

	addressY = addressY & 0x3F; // DB7-6 set to 0
	addressY = addressY | (1 << 6); // DB6 set to 1

	selectDisplaySide(sideSelect);

	sendInstructionToDisplay();
	writeDisplayDataPin();

	waitForTiming();

	displayEnableSignal();
	writeDataBits(addressY);

	waitForTiming();

	displayDisableSignal();
	deselectDisplaySide(sideSelect);
}


// SET PAGE (X ADDRESS)
// 0 0 1 0 1 1 1 AC2 AC1 AC0
// X address (AC0-AC2)
// Writing or reading doesn't change this value
// This is the page
// Each page contains 8 rows
void setAddressX(uint8_t addressX, uint8_t sideSelect){

	if(addressX > 7)
		addressX = 0; // addressX can be 0-7

	addressX = addressX & 0x07; // DB7-3 set to 0
	addressX = addressX | 0xB8; // DB6 set to 0 DB5-3 set to 1

	selectDisplaySide(sideSelect);

	sendInstructionToDisplay();
	writeDisplayDataPin();

	waitForTiming();

	displayEnableSignal();
	writeDataBits(addressX);

	waitForTiming();

	displayDisableSignal();
	deselectDisplaySide(sideSelect);
}

// DISPLAY START LINE (Z ADDRESS)
// 0 0 1 1 AC5 AC4 AC3 AC2 AC1 AC0
// Z address (AC0-AC5)
// Set row (0 - 63) line from where data will be displayed on screen
// RAM content won't be changed
// Set to 0 -> Display start from top
void setAddressZ(uint8_t addressZ, uint8_t sideSelect){
	if(addressZ > 63)
		addressZ = 0; // addressZ can be 0-63

	addressZ = addressZ & 0x3F; // DB7-6 set to 0
	addressZ = addressZ | 0xC0; // DB7-6 set to 1

	selectDisplaySide(sideSelect);

	sendInstructionToDisplay();
	writeDisplayDataPin();

	waitForTiming();

	displayEnableSignal();
	writeDataBits(addressZ);

	waitForTiming();

	displayDisableSignal();
	deselectDisplaySide(sideSelect);
}

// STATUS READ
// 0 1 BUSY 0 ON/OFF RESET 0 0 0 0
// BUSY H -> no instructions are accepted L -> instructions are accepted
// ON/OFF H -> display is OFF L -> display is ON
// RESET H -> display is being initialised, only status read is accepted L -> operating condition
void statusRead(uint8_t sideSelect){
	uint8_t statusBits;

	if(sideSelect != LEFT_AND_RIGHT_SIDE){
		selectDisplaySide(sideSelect);
		GPIOtoInput();

		sendInstructionToDisplay();
		readDisplayDataPin();

		waitForTiming();

		displayEnableSignal();
		waitForTiming();

		statusBits = readDataBits();
		displayDisableSignal();
		deselectDisplaySide(sideSelect);

		GPIOtoOutput();

		if(sideSelect == LEFT_SIDE){
			lcdState.busyStateLeft = (statusBits & 0x80) >> 7;
			lcdState.offStateLeft = (statusBits & 0x20) >> 5;
			lcdState.resetStateLeft = (statusBits & 0x10) >> 4;
		}
		else if(sideSelect == RIGHT_SIDE){
			lcdState.busyStateRight = (statusBits & 0x80) >> 7;
			lcdState.offStateRight = (statusBits & 0x20) >> 5;
			lcdState.resetStateRight = (statusBits & 0x10) >> 4;
		}
	}
}

// WRITE DISPLAY DATA
// 1 0 D7 D6 D5 D4 D3 D2 D1 D0
// Writes data (D0-D7) into display RAM
// After the write, Y address is increased by 1 automatically
void writeDisplay(uint8_t data, uint8_t sideSelect){

	selectDisplaySide(sideSelect);

	sendDataToDisplay();
	writeDisplayDataPin();

	waitForTiming();

	displayEnableSignal();
	writeDataBits(data);

	waitForTiming();

	displayDisableSignal();
	deselectDisplaySide(sideSelect);
}

// READ DISPLAY DATA
// 1 1 D7 D6 D5 D4 D3 D2 D1 D0
// Reads data (D0-D7) from display RAM
// After the read, Y address is increased by 1 automatically
// To read data, two read instructions is needed
// First read is dummy read
uint8_t readDisplay(uint8_t sideSelect){
	uint8_t data = 0;

	if (sideSelect != LEFT_AND_RIGHT_SIDE){
		selectDisplaySide(sideSelect);
		GPIOtoInput();

		receiveDataFromDisplay();
		readDisplayDataPin();

		// First read is dummy read
		waitForTiming();
		displayEnableSignal();
		waitForTiming();
		displayDisableSignal();
		waitForTiming();

		// dummy read

		displayEnableSignal();
		waitForTiming();
		displayDisableSignal();
		waitForTiming();

		data = readDataBits();

		deselectDisplaySide(sideSelect);
		GPIOtoOutput();
	}

	return data;
}

