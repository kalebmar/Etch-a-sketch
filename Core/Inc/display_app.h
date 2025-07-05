#ifndef DISPLAY_APP_H_
#define DISPLAY_APP_H_

#include <stdint.h>

/************************************************************
						Functions
 ************************************************************/

// High-level interface
void displayInit(void);
void clearDisplay(void);
void floodDisplay(void);
void pictureToDisplay(const uint8_t* array);
void animationToDisplay(const uint8_t* image);
void playClown(void);
void sendDisplayData(void);
void loadDisplayData(void);
void cursorOff(uint16_t rawValueX, uint16_t rawValueY);
void drawToDisplay(uint16_t rawValueX, uint16_t rawValueY);
uint8_t storeFromDisplay(uint16_t rawValueX, uint16_t rawValueY);

// Test functions
void drawToDisplayTest(uint8_t xPixel, uint8_t yPixel, uint8_t value);
uint8_t readFromDisplayTest(uint8_t xPixel, uint8_t yPixel);
#endif /* DISPLAY_APP_H_ */
