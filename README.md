# STM32 Baremetal UART Device Driver from Scratch

This project demonstrates building a UART device driver from scratch for STM32F072 microcontrollers without relying on external libraries. The focus is on low-level hardware control using the STM32's hardware peripherals, providing a foundation for embedded systems development and an introduction to writing device drivers in a baremetal environment.

## Features

- **Human-readable UART Functions**: The code provides simple and easy-to-use functions such as uartInit(), uartWriteBuf(), and uartReadByte(), allowing the programmer to quickly set up and interact with UART peripherals.
- **Human-readable GPIO Functions**: The GPIO driver includes functions like gpio_set_mode() for configuring pin modes, and gpio_write() for controlling pin output. These functions simplify GPIO configuration and control, making the code more readable and easier for programmers to use.
- **Command Processing**:
  - Simple command interface to control the state of the LED:
    - `on` - Turns the LED on.
    - `off` - Turns the LED off.
  - Invalid command handling with feedback sent over UART.

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
- The main program blinks an LED and sends a "hi" message over UART1 (115200 baud).
- To view the UART output, you can use one of the following methods:
    - **Logic Analyzer**: Use a logic analyzer to capture the signals on the exposed PA9 (TX) and PA10 (RX) pins.
      
   ![WhatsApp Image 2025-01-25 at 8 11 47 PM](https://github.com/user-attachments/assets/bd43a69e-18e7-47f2-bb4e-fbba8264c7e4)
   ![Screenshot from 2025-01-25 20-12-44](https://github.com/user-attachments/assets/01c446c2-3f34-4932-b4e7-9852e2fd9023)

    - **USB to UART TTL Serial Module**: Connect the PA9 (TX) and PA10 (RX) pins, along with ground, to a USB to UART TTL serial module. Then, use a terminal program(like minicom) to view the UART output by running:
```
sudo minicom -D /dev/ttyUSB0 -b 115200
```
![WhatsApp Image 2025-01-27 at 4 08 04 PM](https://github.com/user-attachments/assets/421869f6-eb13-4221-8c4d-3a4c30c9a62f)

![image](https://github.com/user-attachments/assets/b73a2453-fb87-49c5-b08d-9dcf86a37d72)


Alternatively, you can use other tools like screen to monitor the UART data.

## Future Enhancements
- **USART1 as Virtual COM Port**:
    Upon reviewing the board schematic, I noticed that USART1 was connected to ST-Link. The idea was to use it as a virtual COM port to send UART data directly from the STM32 to a terminal.
    However, this approach did not work as expected because the serial port did not appear on the terminal. Further investigation is needed to figure out why the virtual COM port is not being detected or initialized properly.

![image](https://github.com/user-attachments/assets/2e8a849f-f263-45ad-9f96-3c8c103f0132)

---

## Technical Details and Challenges Faced

### 1. Baremetal Development
The project was developed without relying on any external libraries or HAL (Hardware Abstraction Layer), focusing solely on low-level control of the STM32F072B-DISCO hardware. This approach required:

- Writing direct memory-mapped access to peripherals like GPIO and UART.
- Managing the initialization and configuration of the system clock, UART, and GPIO manually.

This baremetal approach increased the complexity of hardware interfacing but ensured a better understanding of the microcontroller's operation and more efficient resource utilization.

### 2. UART Communication
UART communication was a critical aspect of this project, enabling interaction with the microcontroller via serial communication. The challenges involved:

- **Setting up UART:** Configuring the baud rate, data bits, stop bits, and parity manually. This required careful handling of the UART control registers, ensuring correct timing for transmitting and receiving data.
  
- **Data Reception and Buffering:** Receiving data through UART and storing it in a buffer posed challenges related to buffer overflow. The buffer size had to be managed carefully to prevent data loss, especially when handling a continuous stream of incoming data. This was handled by resetting the buffer index and null-terminating the string when the buffer was full.

- **Interrupt Handling:** Properly handling UART interrupts to manage the flow of data and ensuring smooth operation even under high traffic was important for preventing dropped or corrupted data. Implementing buffer overflow protection helped mitigate this.

### 3. GPIO Control
The project involved controlling an LED connected to a GPIO pin, which was toggled via UART commands. Key challenges here included:

- **GPIO Initialization:** Setting the GPIO pin as an output required enabling the appropriate clock for the port and configuring the pin in the correct mode.
  
- **LED Control:** The logic to read commands from the UART buffer and translate them into GPIO operations had to be carefully designed to prevent errors. Commands like `on` and `off` were mapped to GPIO high and low signals, respectively, ensuring the LED state was updated as intended.

### 4. System Clock Configuration
In this project, the STM32F072 was configured to use the internal High-Speed Internal (HSI) oscillator as the system clock, running at 8 MHz. While this was the default setting, confirming that the clock was correctly initialized and debugging it when there was an issue was challenging.

- **Clock Validation:** Even though the HSI is enabled by default, I used bitwise operations to read and validate the status of the system clock during runtime to ensure there were no issues with the clock configuration. This step involved direct memory access to the RCC (Reset and Clock Control) registers.

### 5. Buffer Overflow Handling
Buffer overflow was a key challenge, especially with limited memory and no dynamic allocation:

- **Overflow Management:** The buffer had to be managed efficiently to handle incoming UART data. In case of buffer overflow, the index was reset, and the buffer was null-terminated. This approach ensured that the data would not be corrupted or lost.
  
- **Improving Robustness:** Initially, I encountered issues where incoming data was overwritten or lost due to improper buffer management. By implementing a mechanism to handle the overflow by resetting the buffer and null-terminating the string, the system became more stable and reliable.

### 6. Error Handling and Feedback
Providing feedback to the user was essential, especially when dealing with incorrect input. This posed a challenge in:

- **Command Validation:** Ensuring that only valid commands (like `on` and `off`) were recognized, and providing error messages like "Invalid Key" if an unknown command was entered.

- **User Experience:** Ensuring the system was responsive and provided clear feedback was crucial, especially in a baremetal environment where the overhead for additional features was minimal.

### Conclusion
The project was a valuable experience in working with low-level hardware and managing the complexities of UART communication, GPIO control, and system clock configuration without relying on external libraries or high-level APIs. The challenges encountered primarily involved managing resources (buffering, interrupts) and ensuring that all hardware peripherals were configured and used correctly. By overcoming these challenges, I gained deeper insights into embedded systems development and improved my problem-solving skills in baremetal environments.

