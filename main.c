#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#define BIT(x) (1UL << (x))
#define PIN(bank, num) ((((bank)-'A')<<8) | (num))
#define PINNO(pin) (pin & 255)
#define PINBANK(pin) (pin >> 8)
//#define SYSTICK_ENABLE
struct gpio {
    volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFRL, AFRH, BRR; 
};
#define GPIO(bank) ((struct gpio *) (0x48000000 + 0x400 * (bank)))
//Enum vlaues are per datasheet: 0,1,2,3...for specifc function
enum { GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG };

//In order to enable a GPIO peripheral, it should be enabled (clocked) via the RCC (Reset and Clock Control) unit.
struct rcc {
    volatile uint32_t CR, CFGR, CIR, APB2RSTR, APB1RSTR, AHBENR, APB2ENR, APB1ENR, BDCR, CSR, AHBRSTR, CFGR2, CFGR3, CR2;
};
#define RCC ((struct rcc *) 0x40021000)
#define RCC_GPIO_CLK_ENABLE(bank)  (RCC->AHBENR |= BIT(17 + (bank)))
#define RCC_GPIO_CLK_DISABLE(bank) (RCC->AHBENR &= ~BIT(17 + (bank)))

#define SYSTICK ((struct systick *) 0xE000E010)
#define UART1 ((struct uart *)0x40013800)
#define FREQ 8000000  // HSI clock frequency is 8 MHz by default

// -------------------------- GPIO Functionality --------------------------
static inline void gpio_set_mode(uint16_t pin, uint8_t mode) {
    // Get the GPIO bank (port) for the specified pin (A, B, C, etc.)
    struct gpio *gpio = GPIO(PINBANK(pin));
    // Extract the pin number (0-15) from the pin identifier (e.g., 'A5' -> 5)
    int n = PINNO(pin);

    RCC_GPIO_CLK_ENABLE(PINBANK(pin));//verify if this works
    // Clear the existing 2-bit mode configuration for the specific pin
    // The 2 bits for each pin are at positions 2*n and 2*n+1 in MODER.
    gpio->MODER &= ~(3U << (n * 2));
    // Set the new mode by OR'ing the correct mode value at the proper position
    // (mode & 3) ensures we only take the lower 2 bits of the mode
    // and then shift them to the correct position for the pin.
    gpio->MODER |= (mode & 3U) << (n * 2);
}

// Define a function to write a value (high/low) to a specific GPIO pin
static inline void gpio_write(uint16_t pin, bool val) {
    // Access the GPIO structure corresponding to the pin's bank (group of pins)
    struct gpio *gpio = GPIO(PINBANK(pin));
    // Write the value to the pin's bit set/reset register (BSRR).
    // The value is shifted left based on whether 'val' is true or false.
    // If 'val' is true (high), it sets the corresponding pin high.
    // If 'val' is false (low), it resets the pin (sets it low).
    gpio->BSRR = (1U << PINNO(pin)) << (val ? 0 : 16);
}

static inline void spin(volatile uint32_t count) {
    while (count--) (void) 0;
}
// -------------------------- UART Functionality --------------------------//
struct uart {
    volatile uint32_t CR1, CR2, CR3, BRR, GTPR, RTOR, RQR, ISR, ICR, RDR, TDR;
}; 

static inline void gpioSetAF(uint16_t pin, uint8_t afNum){

    struct gpio *gpio = GPIO(PINBANK(pin));
    int n = PINNO(pin);

    if(n<8){
        gpio->AFRL &= ~(0xFUL << (n * 4)); //clear AF bit sin AFRL
        gpio->AFRL |= ((uint32_t)afNum << (n *4));
    } else {
        gpio->AFRH &= ~(0xFUL << ((n-8) * 4));
        gpio->AFRH |= ((uint32_t)afNum << ((n-8) * 4));
    }
}

//https://www.st.com/resource/en/datasheet/stm32f072rb.pdf
//Only enables USART1
static inline void uartInit(struct uart *uart , unsigned long baud){
    uint8_t af=1;		//AF1 for USART1 from AF memory mapping
    uint16_t rx=0,tx=0;	

    //Enable Clock for the selected UART
    if(uart == UART1) RCC->APB2ENR |= BIT(14);

    //configure the tx and rx pins
    if(uart == UART1){
        tx = PIN('A',9);
        rx = PIN('A',10);
    }
    //Configure GPIO for UART pins in AF mode
    gpio_set_mode(tx, GPIO_MODE_AF);
    gpioSetAF(tx,af);
    gpio_set_mode(rx, GPIO_MODE_AF);
    gpioSetAF(rx,af);

    //Configure the UART
    uart->CR1 = 0; 			//Disable the UART before configuring
    uart->BRR = FREQ / baud; 		//Set Baud rate (FREQ is the UART clock frequency) 
    uart->CR1 |= BIT(0) | BIT(2) | BIT(3);//Enable UART, RX, TX
}

//Check if UART RX data is ready
static inline int uartReadReady(struct uart *uart){
    return uart->ISR & BIT(5); //Check RXNE(Receiver Not Empty) (bit5 in ISR register)

}

//Read a single Byte from UART
static inline uint8_t uartReadByte(struct uart *uart){
    return (uint8_t)(uart->RDR & 0xFF); //Read 8 bit data from RDR reg
}

//write a single byte to UART 
static inline void uartWriteByte(struct uart *uart, uint8_t byte){
    while((uart->ISR & BIT(7)) == 0); //Wait untill TXE (bit 7 in ISR register) is set
    uart->TDR = byte; //write byte to transmit data register
}

//write a buffer to UART
static inline void uartWriteBuf(struct uart *uart, char *buf, size_t len){
    while(len-- > 0) {
        uartWriteByte(uart, *(uint8_t *) buf++);
    }

}


int main(void){
    uint16_t led=PIN('C',7);
    gpio_set_mode(led, GPIO_MODE_OUTPUT);
    uartInit(UART1,115200);

    char buffer[10]={0};
    uint8_t index=0;
    // Message string
    char message[] = "Send 'on' to turn blue LED on or send 'off' to turn the blue LED off.\r\n";

    // Use sizeof to calculate the length of the message
    uartWriteBuf(UART1, message, sizeof(message) - 1);  // Send message
    // Continuously read data from UART, store it in a buffer, and process it when a complete line (end of input) is received.
    // If the buffer is full, null-terminate the string, reset the index, and start storing new data. This ensures no data is lost
    // and commands are correctly processed (e.g., "on" to turn the LED on and "off" to turn it off).
    for(;;){
        if(uartReadReady(UART1)){
            char c = uartReadByte(UART1);
            if (c == '\r' || c == '\n') {
                // Handle the end of the line and reset buffer
                buffer[index] = '\0';  // Null-terminate the string
                // Check for commands
                if (buffer[0] == 'o' && buffer[1] == 'n') {
                    gpio_write(led, 1);  // Turn LED on
                    uartWriteBuf(UART1, "LED is ON\r\n", sizeof("LED is ON\r\n") - 1);
                } else if (buffer[0] == 'o' && buffer[1] == 'f' && buffer[2] == 'f') {
                    gpio_write(led, 0);  // Turn LED off
                    uartWriteBuf(UART1, "LED is OFF\r\n", sizeof("LED is OFF\r\n") - 1);
                }else{
                    uartWriteBuf(UART1, "Invalid Key\r\n", sizeof("Invalid Key\r\n") - 1);                
                }
                // Reset buffer and index for the next input
                index = 0;
            } else {
                // Store the character in the buffer if it's not a newline
                if (index < sizeof(buffer) - 1) {
                    buffer[index++] = c;  // Add character to buffer
                } else {
                    // Null-terminate and reset index on overflow
                    index = 0;
                }
            }
        }
    }

    return 0;
}

//Startup code
__attribute__((naked, noreturn)) 
    void _reset(void){
        //long is quivalent to uint32_t
        extern long _sbss, _ebss, _sdata, _edata, _srcdata;

        for(long *dst = &_sbss; dst < &_ebss;dst++) *dst=0;
        for(long *dst= &_sdata, *src= &_srcdata;dst< &_edata;) *dst++=*src++;
        main();
        for(;;) (void) 0; //infinite loop incase main() returns

    }

extern void _estack(void); //defined in linker script

//7 standard and 32 STM32-specific handlers
__attribute__((section(".vectors")))
//An array of 38  constant function pointers,
// where each function returns void and takes no arguments.
void (*const vector_table[7+32])(void)={
    _estack,
    _reset
}; 


