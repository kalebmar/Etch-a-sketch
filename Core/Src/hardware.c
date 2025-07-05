#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "hardware.h"
#include "main.h"
#include "display_app.h"

/************************************************************
						Variables
 ************************************************************/

uint32_t BTN1LastPressTime = 0;
uint32_t BTN1LastReleaseTime = 0;
bool BTN1WaitForPress = true;

uint32_t BTN2LastPressTime = 0;
uint32_t BTN2LastReleaseTime = 0;
bool BTN2WaitForPress = true;

/************************************************************
						Functions
 ************************************************************/

// Sets PWM pulse width on TIM3 CH4 based on inverted potentiometer input
uint16_t setDisplayPWMPulse(uint16_t rawInput){
	uint16_t pulse;
	uint16_t counterPeriod = __HAL_TIM_GET_AUTORELOAD(&htim3); // To limit the pulse value to the counter period
	pulse = ((4095 - rawInput) * counterPeriod)/ 4095; // 4095 - rawInput is used because potentiometer is upside down on the board
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, pulse);
	return pulse;
}

// Adds new sample to circular buffer and updates index
void rawNewValue(volatile filterArray* sampleArray, uint8_t length, uint16_t newSample){
	uint8_t i = sampleArray->index;
	if(i == (length - 1)) // Update index before adding new value -> Index points to latest value
		i = 0;
	else
		i = i + 1;

	sampleArray->index = i;
	sampleArray->array[i] = newSample;
}

// Sorts array in ascending order using insertion sort
void arrayInsertSort(uint16_t* array, uint8_t length) {
	 for (uint8_t i = 1; i < length; i++) {
	        uint16_t key = array[i];
	        int j = i - 1;

	        // While left value is bigger, shift right
	        while (j >= 0 && array[j] > key) {
	        	array[j + 1] = array[j];
	            j--;
	        }

	        array[j + 1] = key;
	    }
}

// Filters array by trimming outer quarters, averaging middle half of sorted values
uint16_t filter(uint16_t array[], uint8_t length){
	uint8_t boundary = (length / 4); // The array is trimmed on each side by a quarter of its elements
	uint8_t i;
	uint16_t value;
	uint32_t sum = 0; // ADC max value is 4095 -> 16 samples can be stored in uint16_t
						// but just to be safe
	uint16_t tempArray[length];

	memcpy(tempArray, array, length * sizeof(uint16_t));

	arrayInsertSort(tempArray, length);

	for(i = boundary; i < (length - boundary); i++){
		sum = sum + tempArray[i];
	}

	value = sum/(boundary * 2); // We sum half of the samples -> only divide with half of length

	return value;
}

// Converts ADC value to corresponding pixel row
uint8_t valueToPixelRow(uint16_t value){ // 128x64 px display
	uint8_t pixelRow;

	pixelRow = value / 64; // Value can be 0-63

	return pixelRow;
}


// Converts ADC value to corresponding pixel column
// ADC can't read value over 4037
// Valid values: 128 pixel * 31.5 ->  0-4031
uint8_t valueToPixelCol(uint16_t value){ // 128x64 px display
	uint8_t pixelCol;
	if (value >  4031)
		value = 4031;		 // Saturate value since ADC can read up to 4037
	pixelCol = (value * 128) / 4032; // Divide with 31.5

	return pixelCol;
}


// BTN2
// Toggles cursor visibility and PWM setting based on BTN2 press and long press
void cursorToggle(){
	bool currState;

	currState = HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin);  // BTN2 state is 0 when pressed
	if(currState == 0 && BTN2WaitForPress && (BTN2LastReleaseTime + 400) < timer_100us){
		BTN2LastPressTime = timer_100us;
		BTN2WaitForPress = false;

	}
	else if(currState == 1 && BTN2WaitForPress == false && (BTN2LastPressTime + 400) < timer_100us){
		if(((BTN2LastPressTime + 10000) < timer_100us) && hwStatus.cursorOff){
			hwStatus.setPWM = true;
		}
		else{
			hwStatus.setPWM = false;
			hwStatus.cursorOff = !hwStatus.cursorOff;
			HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, !hwStatus.cursorOff);
		}
		BTN2LastReleaseTime = timer_100us;
		BTN2WaitForPress = true;
	}
}

// BTN1
// Handles BTN1 press/release to toggle erase mode and clear display after long press
void eraseDisplaySignal(){
	bool currState;

	currState = HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin);  // BTN1 state is 0 when pressed


		if(currState == 0 && BTN1WaitForPress && (BTN1LastReleaseTime + 400) < timer_100us){
			BTN1LastPressTime = timer_100us;
			BTN1WaitForPress = false;
		}
		else if(currState == 1 && BTN1WaitForPress == false && (BTN1LastPressTime + 400) < timer_100us){
			if(((BTN1LastPressTime + 10000) < timer_100us) && hwStatus.erase){
				clearDisplay();
			}
			HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, !hwStatus.erase);
			hwStatus.erase = !hwStatus.erase;
			BTN1LastReleaseTime = timer_100us;
			BTN1WaitForPress = true;
		}
}















