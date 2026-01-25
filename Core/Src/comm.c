#include <display_app.h>
#include <stdint.h>
#include "comm.h"

/************************************************************
						Functions
 ************************************************************/

// Executes commands based on input character and resets buffer to '0'
// This should be updated
void keyCommand (uint8_t ch)
{
  switch (ch) {
    case 'r':
      clearDisplay();
      break;
    case 's':
      sendDisplayData();
      break;
    case 'l':
      loadDisplayData();
      break;
    default:
      break;
  }
  buffer = '0';
}
