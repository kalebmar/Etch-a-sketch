#include <stdint.h>
#include "main.h"
#include "display_app.h"
#include "images.h"

/************************************************************
						  Defines
 ************************************************************/

#define CLOWN_REPEAT 8

/************************************************************
						Functions
 ************************************************************/

// Play animation of two clowns
void playClown() {
	clearDisplay();
	for (int j = 0; j < CLOWN_REPEAT; j++) {
		for (int i = 1; i < 9; i++) {
			switch (i) {
			case 1:
				pictureToDisplay(CLOWN1);
				break;
			case 2:
				pictureToDisplay(CLOWN2);
				break;
			case 3:
				pictureToDisplay(CLOWN3);
				break;
			case 4:
				pictureToDisplay(CLOWN4);
				break;
			case 5:
				pictureToDisplay(CLOWN5);
				break;
			case 6:
				pictureToDisplay(CLOWN6);
				break;
			case 7:
				pictureToDisplay(CLOWN7);
				break;
			case 8:
				pictureToDisplay(CLOWN8);
				break;
			}
			HAL_Delay(75);
			clearDisplay();
		}
	}
}
