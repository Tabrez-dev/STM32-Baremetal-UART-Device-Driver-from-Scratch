# STM32 Baremetal UART Device Driver from Scratch

This project demonstrates building a UART device driver from scratch for STM32F072 microcontrollers without relying on external libraries. The focus is on low-level hardware control using the STM32's hardware peripherals, providing a foundation for embedded systems development and an introduction to writing device drivers in a baremetal environment.

## Features
- **Baremetal UART Driver**: Direct control of UART1 without external libraries.
- **GPIO Control**: Configures GPIO pins as alternate function for UART communication.
- **Human-readable UART Functions**: The code provides simple and easy-to-use functions such as uartInit(), uartWriteBuf(), and uartReadByte(), allowing the programmer to quickly set up and interact with UART peripherals.
- **Human-readable GPIO Functions**: The GPIO driver includes functions like gpio_set_mode() for configuring pin modes, and gpio_write() for controlling pin output. These functions simplify GPIO configuration and control, making the code more readable and easier for programmers to use.
- **SysTick Timer (Partially Implemented)**: A simple timer implementation for periodic tasks, though currently under investigation due to issues with initialization causing a hardfault.

## Project Setup
This project is designed for the STM32F072B-DISCO board, which operates with the default 8 MHz HSI oscillator.

### Requirements
- STM32F072B-DISCO Board
- GCC toolchain for ARM
- ST-Link utility for flashing

### Build Instructions
1. Clone the repository:
   ```bash
   git clone https://github.com/Tabrez-dev/STM32-Baremetal-UART-Device-Driver-from-Scratch.git
   ```
2. Compile the project using the provided Makefile:
   ```bash
   make build
   ```
3. Flash the firmware to the board using st link utility:
   ```bash
   make flash
   ```

### Testing
- The main program blinks an LED on pin **C6** and sends a "hi" message over UART1 (115200 baud).
- To view the UART output, you can use one of the following methods:
    - **Logic Analyzer**: Use a logic analyzer to capture the signals on the exposed PA9 (TX) and PA10 (RX) pins.
      
   ![WhatsApp Image 2025-01-25 at 8 11 47 PM](https://github.com/user-attachments/assets/bd43a69e-18e7-47f2-bb4e-fbba8264c7e4)
   ![Screenshot from 2025-01-25 20-12-44](https://github.com/user-attachments/assets/01c446c2-3f34-4932-b4e7-9852e2fd9023)

    - **USB to UART TTL Serial Module**: Connect the PA9 (TX) and PA10 (RX) pins, along with ground, to a USB to UART TTL serial module. Then, use a terminal program to view the UART output by running:
```
cu -l /dev/YOUR_SERIAL_PORT -s 115200
```
Alternatively, you can use other tools like minicom or screen to monitor the UART data.
## Future Enhancements
- **SysTick Timer**:
  - The codebase includes a partially implemented SysTick timer for precise timekeeping and delay functionalities.
  - SysTick initialization currently causes a hardfault; this is under investigation for future debugging and fixes.
  - The `timerExpired` function demonstrates a modular approach to handling periodic events.

- **USART1 as Virtual COM Port**:
    Upon reviewing the board schematic, I noticed that USART1 was connected to ST-Link. The idea was to use it as a virtual COM port to send UART data directly from the STM32 to a terminal.
    However, this approach did not work as expected because the serial port did not appear on the terminal. Further investigation is needed to figure out why the virtual COM port is not being detected or initialized properly.

![image](https://github.com/user-attachments/assets/2e8a849f-f263-45ad-9f96-3c8c103f0132)

- **Why Include Unused Code?**
  - Certain modules like SysTick and `timerExpired` are retained to showcase a forward-thinking design approach.
  - These functions allow for seamless feature extension in future iterations without significant code refactoring.

---
