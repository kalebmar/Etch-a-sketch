#include <display_driver.h>
#include <stdint.h>
#include "comm.h"

void keyCommand (uint8_t ch)
{
  if (ch == 'r')
	  clearDisplay();
  else if(ch == 's')
	  saveDisplay();
  else if(ch == 'l')
	  loadPicture();
  buffer = '0' ;
}
