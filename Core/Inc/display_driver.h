#ifndef INC_DISPLAY_H_
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "main.h"
#include "hardware.h"
#include "clown.h"
#define INC_DISPLAY_H_

/************************************************************
						Defines and enums
 ************************************************************/

//at 48 MHz, 1 cycle ≈ 20.83 ns
#define LCD_TIMING_NOP_COUNT 22
#define CLOWN_REPEAT 8

typedef enum {
    INSTRUCTION,    // 0
	DATA    	    // 1
} DI;

typedef enum {
    NOT_BUSY,    // 0
	BUSY    	 // 1
} busy;

typedef enum {
    DISPLAY_SET,    	// 0
	DISPLAY_RESET    	// 1
} display_reset;

typedef enum {
	LEFT_AND_RIGHT_SIDE,    	// 0
	LEFT_SIDE,    	// 1
	RIGHT_SIDE,    	// 2
} chipSelect;


typedef struct lcdStatus{
	bool resetState;
	bool busyState;
	bool on_offState;
} lcdStatus;

/************************************************************
						Variables
 ************************************************************/

extern lcdStatus lcdState;
extern hwStatusTypedef hwStatus;
extern uint8_t pseudoRAM[1024];
extern UART_HandleTypeDef huart2;

extern const uint8_t opening[];

/************************************************************
						Functions
 ************************************************************/

// Display control
void waitForTiming(void);
void resetDisplay(void);
void setDisplay(void);
void enableDisplay(void);
void disableDisplay(void);

// Display side select
void selectDisplaySide(uint8_t sideSelect);
void deselectDisplaySide(uint8_t sideSelect);

// Data/instruction control
void sendDataToDisplay(void);
void receiveDataFromDisplay(void);
void sendInstructionToDisplay(void);

// Read/write control
void readDisplayDataPin(void);
void writeDisplayDataPin(void);

// Buffer control (not display-specific)
void enableBuffer(void);
void disableBuffer(void);

// Read/write bus
void writeDataBits(uint8_t data);
uint8_t readDataBits(void);

// Display instructions
void displayON(void);
void displayOFF(void);
void setAddressY(uint8_t addressY, uint8_t sideSelect);
void setAddressX(uint8_t addressX, uint8_t sideSelect);
void setAddressZ(uint8_t addressZ, uint8_t sideSelect);
void statusRead(uint8_t sideSelect);
void writeDisplay(uint8_t data, uint8_t sideSelect);
uint8_t readDisplay(uint8_t sideSelect);

// High-level interface
void displayInit(void);
void clearDisplay(void);
void floodDisplay(void);
void pictureToDisplay(const unsigned char array[]);
void playClown(void);
void saveDisplay(void);
void loadPicture(void);
void cursorOff(uint16_t rawValueX, uint16_t rawValueY);
void drawToDisplay(uint16_t rawValueX, uint16_t rawValueY);
uint8_t storeFromDisplay(uint16_t rawValueX, uint16_t rawValueY);

// Test functions
void drawToDisplayTest(uint8_t xPixel, uint8_t yPixel, uint8_t value);
uint8_t readFromDisplayTest(uint8_t xPixel, uint8_t yPixel);

#endif /* INC_DISPLAY_H_ */
