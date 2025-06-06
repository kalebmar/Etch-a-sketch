#include <display_driver.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "main.h"

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
						Low level functions
 ************************************************************/

// Wait LCD_TIMING_NOP_COUNT clock period
// For NT7108 wait needs to be at least 450ns
void waitForTiming(){
	for(uint8_t i = 0; i < LCD_TIMING_NOP_COUNT; i++)
		__NOP();
}
// Display reset is active low
// /RST signal on PB9
void resetDisplay(){
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
}

// Display reset is active low
// /RST signal on PB9
// Reset time is at least 1 us
// After reset Display is OFF
// Z address set to 0
void setDisplay(){
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
}

// Display enable is active high
// Enable signal on PC8
void enableDisplay(){
	HAL_GPIO_WritePin(LCD_ENABLE_GPIO_Port, LCD_ENABLE_Pin, GPIO_PIN_SET);
}

// Display enable is active high
// Enable signal on PC8
void disableDisplay(){
	HAL_GPIO_WritePin(LCD_ENABLE_GPIO_Port, LCD_ENABLE_Pin, GPIO_PIN_RESET);
}

// Data or instruction H: Data  L : Instruction
// Data or instruction signal on PB8
void sendDataToDisplay(){
	HAL_GPIO_WritePin(LCD_DI_GPIO_Port, LCD_DI_Pin, GPIO_PIN_SET);
}
void receiveDataFromDisplay(){
	HAL_GPIO_WritePin(LCD_DI_GPIO_Port, LCD_DI_Pin, GPIO_PIN_SET);
}

// Data or instruction H: Data  L : Instruction
// Data or instruction signal on PB8
void sendInstructionToDisplay(){
	HAL_GPIO_WritePin(LCD_DI_GPIO_Port, LCD_DI_Pin, GPIO_PIN_RESET);
}

// Display left side select CS1 is active L
// CS1 signal on PB11
// Display right side select CS2 is active L
// CS2 signal on PC5
// 0 both selected
// 1 CS1 selected
// 2 CS2 selected
void selectDisplaySide(uint8_t sideSelect){
	if(sideSelect == LEFT_AND_RIGHT_SIDE){
		HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_RESET);
	}
	else if(sideSelect == LEFT_SIDE)
		HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_RESET);
	else if(sideSelect == RIGHT_SIDE)
		HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_RESET);
}

// Display left side select CS1 is active L
// CS1 signal on PB11
// Display right side select CS2 is active L
// CS2 signal on PC5
// 0 both selected -> only use it for write / instructions
// 1 CS1 selected
// 2 CS2 selected
void deselectDisplaySide(uint8_t sideSelect){
	if(sideSelect == LEFT_AND_RIGHT_SIDE){
		HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_SET);
	}
	else if(sideSelect == LEFT_SIDE)
		HAL_GPIO_WritePin(LCD_CS1_GPIO_Port, LCD_CS1_Pin, GPIO_PIN_SET);
	else if(sideSelect == RIGHT_SIDE)
		HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_SET);
}


// Read or write data to display MCU read: H MCU write: L
// Read Write signal on PC6
void readDisplayDataPin(){
	HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, GPIO_PIN_SET);
}

// Read or write data to display MCU read: H MCU write: L
// Read Write signal on PC6
void writeDisplayDataPin(){
	HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, GPIO_PIN_RESET);
}


// Enables the Output of the buffer IC NOT related to display
// _245OE signal is active L
// _245OE signal on PC12
void enableBuffer(){
	HAL_GPIO_WritePin(_245OE_GPIO_Port, _245OE_Pin, GPIO_PIN_RESET);
}

// Disables the Output of the buffer IC NOT related to display
// _245OE signal is active L
// _245OE signal on PC12
void disableBuffer(){
	HAL_GPIO_WritePin(_245OE_GPIO_Port, _245OE_Pin, GPIO_PIN_SET);
}

// Sets the pins of DB7-DB0
void writeDataBits(uint8_t data){
	uint16_t maskedGPIOA_ODR = GPIOA->ODR & 0xE01F; // PA15-13 and PA4-0 unchanged -> PA12-5 erased
	uint8_t DB3 = data & 0x08;
	uint8_t DB4 = data & 0x10;
	// Because DB4 and DB3 is in wrong order in the ports we have to swap them
	// We only need to swap them if they have different values
	// DB4 0 DB3 0 -> no need for swap
	// DB4 0 DB3 1 -> invert bits
	// DB4 1 DB3 0 -> invert bits
	// DB4 1 DB3 1 -> no need for swap
	if(DB3 != (DB4 >> 1)){
		data = data & 0xE7; // Mask with 1110 0111 to erase DB4 and DB3
		data = data | (DB3 << 1); // Move DB3 to DB4
		data = data | (DB4 >> 1); // Move DB4 to DB3
	}
	GPIOA->ODR = maskedGPIOA_ODR | (data << 5);
}

// Read the pins of DB7-DB0
uint8_t readDataBits(){
	uint32_t gpio = GPIOA->IDR;
	uint16_t maskedGPIOA_IDR = GPIOA->IDR & 0x1FE0; // PA15-13 and PA4-0 erased -> PA12-5 unchanged
	uint8_t data = maskedGPIOA_IDR >> 5;
	uint8_t DB3 = data & 0x08;
	uint8_t DB4 = data & 0x10;
	// Because DB4 and DB3 is in wrong order in the ports we have to swap them
	// We only need to swap them if they have different values
	// DB4 0 DB3 0 -> no need for swap
	// DB4 0 DB3 1 -> invert bits
	// DB4 1 DB3 0 -> invert bits
	// DB4 1 DB3 1 -> no need for swap
	if(DB3 != (DB4 >> 1)){
		data = data & 0xE7; // Mask with 1110 0111 to erase DB4 and DB3
		data = data | (DB3 << 1); // Move DB3 to DB4
		data = data | (DB4 >> 1); // Move DB4 to DB3
	}
	return data;
}

// Sets PA12-PA5 to GPIO input
void GPIOtoInput(){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

		GPIO_InitStruct.Pin = 	LCD_DB0_Pin | LCD_DB1_Pin | LCD_DB2_Pin |
								LCD_DB3_Pin | LCD_DB4_Pin | LCD_DB5_Pin |
								LCD_DB6_Pin | LCD_DB7_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

// Sets PA12-PA5 to GPIO input
void GPIOtoOutput(){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

		GPIO_InitStruct.Pin = 	LCD_DB0_Pin | LCD_DB1_Pin | LCD_DB2_Pin |
								LCD_DB3_Pin | LCD_DB4_Pin | LCD_DB5_Pin |
								LCD_DB6_Pin | LCD_DB7_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/************************************************************
						Display instructions
 ************************************************************/

// Instruction bits from MSB to LSB
// RS R/W DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0
// Renamed to
// DI RW DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0

// DISPLAY ON/OFF
// 0 0 0 0 1 1 1 1 1 D
// Display data when D H
// If D L data still remains in RAM
void displayON(){
	selectDisplaySide(LEFT_AND_RIGHT_SIDE);

	sendInstructionToDisplay();
	writeDisplayDataPin();

	waitForTiming();

	enableDisplay();
	writeDataBits(0b00111111); // Last bit H

	waitForTiming();

	deselectDisplaySide(LEFT_AND_RIGHT_SIDE);
	disableDisplay();

}

void displayOFF(){
	selectDisplaySide(LEFT_AND_RIGHT_SIDE);

	sendInstructionToDisplay();
	writeDisplayDataPin();

	waitForTiming();

	enableDisplay();
	writeDataBits(0b00111110); // Last bit L

	waitForTiming();

	deselectDisplaySide(LEFT_AND_RIGHT_SIDE);
	disableDisplay();
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

	enableDisplay();
	writeDataBits(addressY);

	waitForTiming();

	deselectDisplaySide(sideSelect);
	disableDisplay();
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

	enableDisplay();
	writeDataBits(addressX);

	waitForTiming();

	deselectDisplaySide(sideSelect);
	disableDisplay();
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

	enableDisplay();
	writeDataBits(addressZ);

	waitForTiming();

	deselectDisplaySide(sideSelect);
	disableDisplay();
}

// STATUS READ
// 0 1 BUSY 0 ON/OFF RESET 0 0 0 0
// BUSY H -> no instructions are accepted L -> instructions are accepted
// ON/OFF H -> display is OFF L -> display is ON
// RESET H -> display is being initialised, only status read is accepted L -> operating condition

void statusRead(uint8_t sideSelect){
	uint8_t statusBits;

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = 	LCD_DB0_Pin | LCD_DB1_Pin | LCD_DB2_Pin |
							LCD_DB3_Pin | LCD_DB4_Pin | LCD_DB5_Pin |
							LCD_DB6_Pin | LCD_DB7_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	if(sideSelect != LEFT_AND_RIGHT_SIDE)
	selectDisplaySide(sideSelect);

	sendInstructionToDisplay();
	readDisplayDataPin();

	waitForTiming();

	enableDisplay();
	waitForTiming();

	statusBits = readDataBits();
	disableDisplay();
	deselectDisplaySide(sideSelect);

	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	lcdState.busyState = (statusBits & 0x80) >> 7;
	lcdState.on_offState = (statusBits & 0x20) >> 5;
	lcdState.resetState = (statusBits & 0x10) >> 4;
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

	enableDisplay();
	writeDataBits(data);

	waitForTiming();

	disableDisplay();
	deselectDisplaySide(sideSelect);
}

// READ DISPLAY DATA
// 1 1 D7 D6 D5 D4 D3 D2 D1 D0
// Reads data (D0-D7) from display RAM
// After the read, Y address is increased by 1 automatically
// To read data, two read instructions is needed
// First read is dummy read

uint8_t readDisplay(uint8_t sideSelect){
	uint8_t data;

	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
	GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
	GPIO_PIN_11 | GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	if (sideSelect != LEFT_AND_RIGHT_SIDE)
		selectDisplaySide(sideSelect);

	receiveDataFromDisplay();
	readDisplayDataPin();

	waitForTiming();
	enableDisplay();
	waitForTiming();
	disableDisplay();
	waitForTiming();

	enableDisplay();
	waitForTiming();
	data = readDataBits();
	disableDisplay();

	deselectDisplaySide(sideSelect);

	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	return data;
}

//*************************************************
// Interface for display

void displayInit(){

	enableBuffer();

	selectDisplaySide(LEFT_AND_RIGHT_SIDE);
	waitForTiming();

	resetDisplay();

	waitForTiming();

	setDisplay();

	// Reset time is at least 1 us
	for(uint8_t i = 0; i < 4; i++)
	waitForTiming();

	displayON();
	waitForTiming();

	deselectDisplaySide(LEFT_AND_RIGHT_SIDE);
	waitForTiming();

	//Z address is set to 0 by default after RESET
	clearDisplay();
	setDisplayPWMPulse(1000);
}

void clearDisplay(){
	memset(pseudoRAM, 0, 1024); // Clear pseudoRAM

	for(uint8_t i = 0; i < 8; i++){
		setAddressX(i, LEFT_AND_RIGHT_SIDE); // In one page there is 8px

		setAddressY(0, LEFT_AND_RIGHT_SIDE);  // Display use y for horizontal x for vertical

		for(uint8_t j = 0; j < 64; j++){
		writeDisplay(0, LEFT_SIDE);
		writeDisplay(0, RIGHT_SIDE);
		}
	}
}

void saveDisplay(){
	char msg[10];
	displayOFF();
	for(int i = 0; i < 1024; i++){
	//HAL_UART_Transmit(&huart2, &pseudoRAM[i], 1, HAL_MAX_DELAY);
		sprintf(msg,"%hu\r\n", pseudoRAM[i]);
	HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
	}
	displayON();
}

void loadPicture(){
	displayOFF();
	HAL_UART_Receive(&huart2, pseudoRAM, sizeof(pseudoRAM), HAL_MAX_DELAY);
	displayON();
	clearDisplay();
	pictureToDisplay(opening);
}

void floodDisplay(){

	for(uint8_t i = 0; i < 8; i++){
		setAddressX(i, LEFT_AND_RIGHT_SIDE); // In one page there is 8px
		setAddressY(0, LEFT_AND_RIGHT_SIDE);   // Display use y for horizontal x for vertical

		for(uint8_t j = 0; j < 64; j++){
		writeDisplay(0xFF, LEFT_AND_RIGHT_SIDE);
		}
	}

}


void drawToDisplay(uint16_t rawValueX, uint16_t rawValueY){
	uint8_t xPixel = valueToPixelCol(rawValueX);
	uint8_t yPixel = valueToPixelRow(rawValueY);
	uint16_t index = xPixel + (yPixel / 8) * 128;


	pseudoRAM[index] = pseudoRAM[index] | (1 << (yPixel % 8));
	if(xPixel < 64){ // 0-63 left side CS1
		setAddressX(yPixel / 8, LEFT_SIDE); // In one page there is 8px
		setAddressY(xPixel, LEFT_SIDE);  // Display use y for horizontal x for vertical
		writeDisplay(pseudoRAM[index], LEFT_SIDE);
	}
	else{ // 64-127 right side CS2
		setAddressX(yPixel / 8, RIGHT_SIDE); // In one page there is 8px
		setAddressY(xPixel - 64, RIGHT_SIDE);  // Display use y for horizontal x for vertical
		writeDisplay(pseudoRAM[index], RIGHT_SIDE);
	}

}



void cursorOff(uint16_t rawValueX, uint16_t rawValueY){
	uint8_t data;
	uint8_t xPixel = valueToPixelCol(rawValueX);
	uint8_t yPixel = valueToPixelRow(rawValueY);
	uint16_t index = xPixel + (yPixel / 8) * 128;

	if(prevXpixel < 64){ // 0-63 left side CS1
				setAddressX(prevYpixel / 8, LEFT_SIDE); // In one page there is 8px
				setAddressY(prevXpixel, LEFT_SIDE); // Display use y for horizontal x for vertical
				writeDisplay(pseudoRAM[prevXpixel + (prevYpixel / 8) * 128], LEFT_SIDE);
				}
			else{ // 64-127 right side CS2
					setAddressX(prevYpixel / 8, RIGHT_SIDE); // In one page there is 8px
					setAddressY(prevXpixel - 64, RIGHT_SIDE); // Display use y for horizontal x for vertical
					writeDisplay(pseudoRAM[prevXpixel + (prevYpixel / 8) * 128], RIGHT_SIDE);
				}


	data = pseudoRAM[index] | (1 << (yPixel % 8));
		if(xPixel < 64){ // 0-63 left side CS1
				setAddressX(yPixel / 8, LEFT_SIDE); // In one page there is 8px
				setAddressY(xPixel, LEFT_SIDE); // Display use y for horizontal x for vertical
				writeDisplay(data, LEFT_SIDE);
			}
		else{ // 64-127 right side CS2
				setAddressX(yPixel / 8, RIGHT_SIDE); // In one page there is 8px
				setAddressY(xPixel - 64, RIGHT_SIDE); // Display use y for horizontal x for vertical
				writeDisplay(data, RIGHT_SIDE);
			}

	prevXpixel = xPixel;
	prevYpixel = yPixel;

}


uint8_t readFromDisplayTest(uint8_t xPixel, uint8_t yPixel){
	uint8_t data;

	if(xPixel < 64){ // 0-63 left side CS1
		setAddressX(yPixel / 8, LEFT_SIDE); // In one page there is 8px
		setAddressY(xPixel, LEFT_SIDE); // Display use y for vertical x for horizontal
		data = readDisplay(LEFT_SIDE);
	}
	else{ // 64-127 right side CS2
		xPixel = xPixel - 64;

		setAddressX(yPixel / 8, RIGHT_SIDE); // In one page there is 8px
		setAddressY(xPixel, RIGHT_SIDE); // Display use y for vertical x for horizontal
		data = readDisplay(RIGHT_SIDE);
	}
	return data;
}

void drawToDisplayTest(uint8_t xPixel, uint8_t yPixel, uint8_t value){

	if(xPixel < 64){ // 0-63 left side CS1
		setAddressX(yPixel / 8, LEFT_SIDE); // In one page there is 8px
		setAddressY(xPixel, LEFT_SIDE); // Display use y for vertical x for horizontal
		writeDisplay(value, LEFT_SIDE);
	}
	else{ // 64-127 right side CS2
		xPixel = xPixel - 64;

		setAddressX(yPixel / 8, RIGHT_SIDE); // In one page there is 8px
		setAddressY(xPixel, RIGHT_SIDE); // Display use y for vertical x for horizontal
		writeDisplay(value, RIGHT_SIDE);
	}
}

void pictureToDisplay(const uint8_t array[]){
uint8_t xPixel;
uint8_t yPixel;
uint8_t data;


for(yPixel = 0; yPixel < 8; yPixel++)
	{
		for(xPixel = 0; xPixel < 128; xPixel ++)
		{
			if(xPixel < 64){ // 0-63 left side CS1
				setAddressX(yPixel, LEFT_SIDE); // In one page there is 8px
				data = array[xPixel + 128 * yPixel];
				writeDisplay(data, LEFT_SIDE);
				}
			else{ // 64-127 right side CS2

				setAddressX(yPixel, RIGHT_SIDE); // In one page there is 8px
				data = array[xPixel + 128 * yPixel];
				writeDisplay(data, RIGHT_SIDE);
				}
		}
	}
}


void playClown(){
	clearDisplay();
	for(int j = 0; j < CLOWN_REPEAT; j++){
	for(int i = 1; i < 9; i++){
		switch(i){
		case 1: pictureToDisplay(CLOWN1);
					break;
		case 2: pictureToDisplay(CLOWN2);
					break;
		case 3: pictureToDisplay(CLOWN3);
					break;
		case 4: pictureToDisplay(CLOWN4);
					break;
		case 5: pictureToDisplay(CLOWN5);
					break;
		case 6: pictureToDisplay(CLOWN6);
					break;
		case 7: pictureToDisplay(CLOWN7);
					break;
		case 8: pictureToDisplay(CLOWN8);
					break;
		}
		HAL_Delay(75);
		clearDisplay();
	}
	}
}









