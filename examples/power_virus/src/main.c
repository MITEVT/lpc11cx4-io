

#include "chip.h"
#include <string.h>

const uint32_t OscRateIn = 0;

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define UART_BAUD_RATE 115200

#define LED0 2, 8
#define LED1 2, 10

#define UART_RX_BUFFER_SIZE 8

volatile uint32_t msTicks;

static char uart_rx_buf[UART_RX_BUFFER_SIZE];

#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
	#define DEBUG_Print(str) Chip_UART_SendBlocking(LPC_USART, str, strlen(str))
	#define DEBUG_Write(str, count) Chip_UART_SendBlocking(LPC_USART, str, count)
#else
	#define DEBUG_Print(str)
	#define DEBUG_Write(str, count) 
#endif

/*****************************************************************************
 * Private functions
 ****************************************************************************/

void SysTick_Handler(void) {
	msTicks++;
}

static void LED_Init(uint8_t port, uint8_t pin) {
	Chip_GPIO_WriteDirBit(LPC_GPIO, port, pin, true);
	Chip_GPIO_SetPinState(LPC_GPIO, port, pin, false);

}

static void LED_Write(uint8_t port, uint8_t pin, uint8_t val) {
	Chip_GPIO_SetPinState(LPC_GPIO, port, pin, val);
}


int main(void)
{

	SystemCoreClockUpdate();

	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	LPC_SYSCTL->CLKOUTSEL = 0x03; 		// Main CLK (Core CLK) Out
	LPC_SYSCTL->CLKOUTUEN = 0x00; 		// Toggle Update CLKOUT Source
	LPC_SYSCTL->CLKOUTUEN = 0x01;
	while(!(LPC_SYSCTL->CLKOUTUEN & 0x1)); // Wait until updated
	LPC_SYSCTL->CLKOUTDIV = 0x04; 		// No division

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_1, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_OPENDRAIN_EN)); /*CLKOUT*/

	//---------------
	//UART
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, UART_BAUD_RATE);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);
	//---------------


	LPC_SYSCTL->SYSAHBCLKCTRL |= 0x7BFE0; // Enable all peripheral clocks
	LPC_SYSCTL->PRESETCTRL |= 0xF; 			// Disable peripheral resets
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_5, (IOCON_FUNC2|IOCON_MODE_INACT)); // protect input
	LPC_GPIO[1].DIR |= 0xFFF;
	LPC_GPIO[1].DATA[0x3FFC] = 0xAAA;
	LPC_GPIO[2].DIR |= 0xFFF;
	LPC_GPIO[2].DATA[0x3FFC] = 0x555;
	LPC_GPIO[3].DIR |= 0xFFF;
	LPC_GPIO[3].DATA[0x3FFC] = 0xAAA;


	DEBUG_Print("Started up\n\r");

	LED_Init(LED0);
	LED_Init(LED1);

	LED_Write(LED0, true);

	while (1) {
		uint8_t count;
		if ((count = Chip_UART_Read(LPC_USART, uart_rx_buf, UART_RX_BUFFER_SIZE)) != 0) {
				DEBUG_Write(uart_rx_buf, count);
		}

		LPC_GPIO[1].DATA[0x3FFC] ^= 0xFFF;
		LPC_GPIO[2].DATA[0x3FFC] ^= 0xFFF;
		LPC_GPIO[3].DATA[0x3FFC] ^= 0xFFF;
	}
}
