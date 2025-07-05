#ifndef INC_HARDWARE_H_
#define INC_HARDWARE_H_

#include <stdbool.h>
#include <stdint.h>
#include "main.h"

/************************************************************
						Defines and enums
 ************************************************************/

#define SAMPLE_SIZE 32 // Use power of twos

typedef struct filterArray{
	uint8_t index;
	uint8_t length;
	uint16_t array[SAMPLE_SIZE];
} filterArray;

typedef struct hwStatusStruct{
	bool clown;
	bool cursorOff;
	bool erase;
	bool setPWM;
} hwStatusTypedef;

/************************************************************
						Variables
 ************************************************************/


extern volatile uint32_t timer_100us;

extern hwStatusTypedef hwStatus;
extern TIM_HandleTypeDef htim3;

/************************************************************
						Functions
 ************************************************************/

// PWM
uint16_t setDisplayPWMPulse(uint16_t rawInput);

// New value insert and filters

void rawNewValue(volatile filterArray* sampleArray, uint8_t length, uint16_t newSample);
void arrayInsertSort(uint16_t* array, uint8_t length);
uint16_t filter(uint16_t array[], uint8_t length);

// Convert ADC value to pixel value
uint8_t valueToPixelRow(uint16_t value);
uint8_t valueToPixelCol(uint16_t value);

// Display control
void cursorToggle();
void eraseDisplaySignal();

#endif /* INC_HARDWARE_H_ */
