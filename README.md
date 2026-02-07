# Etch-a-sketch with 128x64 LCD Board (STM32 Nucleo)

This project implements an Etch-A-Sketch–style drawing device using an STM32 NUCLEO-F091RC development board and a 128×64 LCD.

The device allows the user to draw on the display using two potentiometers. The board’s buttons provide functions such as clearing the screen, adjusting the display contrast via a PWM signal, “lifting” the cursor (so drawing can be paused while moving), and playing back pre-recorded animations. In addition, the device can communicate with a PC host application through a UART interface, enabling images to be sent between the display and the computer.

All aspects of the project were completed individually, including the hardware design, PCB layout, soldering, component selection, as well as the development of the display driver, main firmware functions, and the PC host application.

---

## Main Board Features

- **Low-Level Display Driver:**  
  Custom bare-metal driver for the NT7108 controller, directly manipulating GPIO registers instead of using HAL libraries to achieve maximum performance.

- **Drawing Control:**  
  Cursor movement is controlled by two analog potentiometers read via the ADC. Median and averaging filters are applied to the sampled input values to ensure more accurate and stable cursor movement.

- **Clear Display:**  
  Pressing the left, debounced button initiates a reset sequence. Holding the button for 1.5 seconds clears the display. A red LED provides visual feedback on the current state.

- **Cursor Lift:**  
  Pressing the right button lifts the cursor, allowing movement without drawing and enabling non-continuous lines. A blue LED indicates the cursor state.

- **Display Brightness Adjustment:**  
  Holding the right button for 1.5 seconds enters the PWM brightness adjustment mode.

- **PseudoRAM Frame Buffer:**  
  A pseudo RAM buffer is used instead of reading back the display content, ensuring faster screen updates.

- **UART Communication:**  
  The device communicates with a PC host over UART and is capable of sending and receiving image data.

---

## Technical Implementation

### 1. NT7108 Driver (display_driver.c and display_driver.h)

To achieve the high refresh rates required for smooth animation, the driver was implemented using direct register access (e.g. GPIOx_ODR, GPIOx_IDR), avoiding higher-level abstractions.

Precise instruction-level delays (nop) are used to meet the 450 ns E signal timing requirements of the display.
