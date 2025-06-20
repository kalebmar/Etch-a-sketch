#include <stdint.h>
#include "main.h"
#include "display_app.h"
#include "images.h"


/************************************************************
						Functions
 ************************************************************/

// Play animation of two clowns
void playClown() {
	clearDisplay();
	for (int j = 0; j < CLOWN_IMAGE_NUMBER - 1; j++) {
		 animationToDisplay(CLOWN[j], CLOWN[j + 1]);
	}
}
