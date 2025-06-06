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
  if (ch == 'r')
	  clearDisplay();
  else if(ch == 's')
	  sendDisplayData();
  else if(ch == 'l')
	  loadDisplayData();
  buffer = '0' ;
}
