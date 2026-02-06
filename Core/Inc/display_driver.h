#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

/************************************************************
						Defines and enums
 ************************************************************/

//at 48 MHz, 1 cycle ≈ 20.83 ns
#define LCD_TIMING_NOP_COUNT 100
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
	LEFT_AND_RIGHT_SIDE,    // 0
	LEFT_SIDE,    			// 1
	RIGHT_SIDE,    			// 2
} chipSelect;


typedef struct lcdStatus{
	bool resetStateLeft;
	bool busyStateLeft;
	bool on_offStateLeft;
	bool resetStateRight;
	bool busyStateRight;
	bool on_offStateRight;
} lcdStatus;

/************************************************************
						Variables
 ************************************************************/

extern UART_HandleTypeDef huart2;
extern lcdStatus lcdState;

extern uint8_t pseudoRAM[1024];

extern const uint8_t opening[];

/************************************************************
						Functions
 ************************************************************/

// Display instructions
void displayON(void);
void displayOFF(void);
void setAddressY(uint8_t addressY, uint8_t sideSelect);
void setAddressX(uint8_t addressX, uint8_t sideSelect);
void setAddressZ(uint8_t addressZ, uint8_t sideSelect);
void statusRead(uint8_t sideSelect);
void writeDisplay(uint8_t data, uint8_t sideSelect);
uint8_t readDisplay(uint8_t sideSelect);

/************************************************************
						Low level functions
 ************************************************************/

// Wait LCD_TIMING_NOP_COUNT clock period
// For NT7108 wait needs to be at least 450ns
static inline __attribute__((always_inline)) void waitForTiming(){
	for(uint8_t i = 0; i < LCD_TIMING_NOP_COUNT; i++)
		__NOP();
}

// Display reset is active low
// /RST signal on PB9
static inline __attribute__((always_inline)) void resetDisplay(){
	LCD_RST_GPIO_Port->BRR = LCD_RST_Pin;
}

// Display reset is active low
// /RST signal on PB9
// Reset time is at least 1 us
// After reset Display is OFF
// Z address set to 0
static inline __attribute__((always_inline)) void setDisplay(){
	LCD_RST_GPIO_Port->BSRR = LCD_RST_Pin;
}

// Display enable is active high
// Enable signal on PC8
static inline __attribute__((always_inline)) void displayEnableSignal(){
	LCD_ENABLE_GPIO_Port->BSRR = LCD_ENABLE_Pin;
}

// Display enable is active high
// Enable signal on PC8
static inline __attribute__((always_inline)) void displayDisableSignal(){
	LCD_ENABLE_GPIO_Port->BRR = LCD_ENABLE_Pin;
}

// Data or instruction H: Data  L : Instruction
// Data or instruction signal on PB8
static inline __attribute__((always_inline)) void sendDataToDisplay(){
	LCD_DI_GPIO_Port->BSRR = LCD_DI_Pin;
}

static inline __attribute__((always_inline)) void receiveDataFromDisplay(){
	LCD_DI_GPIO_Port->BSRR = LCD_DI_Pin;
}

// Data or instruction H: Data  L : Instruction
// Data or instruction signal on PB8
static inline __attribute__((always_inline)) void sendInstructionToDisplay(){
	LCD_DI_GPIO_Port->BRR = LCD_DI_Pin;
}

// Display left side select CS1 is active L
// CS1 signal on PB11
// Display right side select CS2 is active L
// CS2 signal on PC5
// 0 both selected
// 1 CS1 selected
// 2 CS2 selected
static inline __attribute__((always_inline)) void selectDisplaySide(uint8_t sideSelect){
	if(sideSelect == LEFT_AND_RIGHT_SIDE){
		LCD_CS1_GPIO_Port->BRR = LCD_CS1_Pin;
		LCD_CS2_GPIO_Port->BRR = LCD_CS2_Pin;
	}
	else if(sideSelect == LEFT_SIDE)
		LCD_CS1_GPIO_Port->BRR = LCD_CS1_Pin;
	else if(sideSelect == RIGHT_SIDE)
		LCD_CS2_GPIO_Port->BRR = LCD_CS2_Pin;
}

// Display left side select CS1 is active L
// CS1 signal on PB11
// Display right side select CS2 is active L
// CS2 signal on PC5
// 0 both selected -> only use it for write / instructions
// 1 CS1 selected
// 2 CS2 selected
static inline __attribute__((always_inline)) void deselectDisplaySide(uint8_t sideSelect){
	if(sideSelect == LEFT_AND_RIGHT_SIDE){
		LCD_CS1_GPIO_Port->BSRR = LCD_CS1_Pin;
		LCD_CS2_GPIO_Port->BSRR = LCD_CS2_Pin;
	}
	else if(sideSelect == LEFT_SIDE)
		LCD_CS1_GPIO_Port->BSRR = LCD_CS1_Pin;
	else if(sideSelect == RIGHT_SIDE)
		LCD_CS2_GPIO_Port->BSRR = LCD_CS2_Pin;
}


// Read or write data to display MCU read: H MCU write: L
// Read Write signal on PC6
static inline __attribute__((always_inline)) void readDisplayDataPin(){
	LCD_RW_GPIO_Port->BSRR = LCD_RW_Pin;
}

// Read or write data to display MCU read: H MCU write: L
// Read Write signal on PC6
static inline __attribute__((always_inline)) void writeDisplayDataPin(){
	LCD_RW_GPIO_Port->BRR = LCD_RW_Pin;
}

// Enables the Output of the buffer IC NOT related to display
// _245OE signal is active L
// _245OE signal on PC12
static inline __attribute__((always_inline)) void enableBuffer(){
	_245OE_GPIO_Port->BRR = _245OE_Pin;
}

// Disables the Output of the buffer IC NOT related to display
// _245OE signal is active L
// _245OE signal on PC12
static inline __attribute__((always_inline)) void disableBuffer(){
	_245OE_GPIO_Port->BSRR = _245OE_Pin;
}

// Sets the pins of DB7-DB0
static inline __attribute__((always_inline)) void writeDataBits(uint8_t data){
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
static inline __attribute__((always_inline)) uint8_t readDataBits(){
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
static inline __attribute__((always_inline)) void GPIOtoInput(){
		GPIOA->MODER = GPIOA->MODER & 0xFC0003FF; // PA5-12 set to 00 -> Input mode
		GPIOA->PUPDR = GPIOA->PUPDR & 0xFC0003FF; // PA5-12 set to 00 -> No pull-up, pull-down
}

// Sets PA12-PA5 to GPIO input
static inline __attribute__((always_inline)) void GPIOtoOutput(){
	GPIOA->MODER = (GPIOA->MODER & 0xFC0003FF) | 0x00155400; // PA5-12 set to 01 -> General purpose output mode
	GPIOA->OTYPER = GPIOA->OTYPER & 0x0000E01F; // PA5-12 set to 01 -> Output push-pull
	GPIOA->OSPEEDR = GPIOA->OSPEEDR | 0x03FFFC00; // PA5-12 set to 11 -> High speed
	GPIOA->PUPDR = GPIOA->PUPDR & 0xFC0003FF; // PA5-12 set to 00 -> No pull-up, pull-down
}

#endif /* INC_DISPLAY_H_ */
