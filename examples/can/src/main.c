

#include "chip.h"
#include "util.h"
#include <string.h>
#include "can.h"
#include <stdlib.h>

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define TEST_CCAN_BAUD_RATE 500000

#define LED_PORT 0
#define LED_PIN 7

#define UART_RX_BUFFER_SIZE 8

const uint32_t OscRateIn = 12000000;


volatile uint32_t msTicks;

CCAN_MSG_OBJ_T rx_buffer;
static char str[100];
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


int main(void)
{

	SystemCoreClockUpdate();

	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}


	//---------------
	//UART
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 57600);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);
	//---------------

	DEBUG_Print("Started up\n\r");

	CAN_Init(TEST_CCAN_BAUD_RATE);
	uint32_t CAN_error_ID;

    uint32_t nxtMsg = msTicks+2000;

	while (1) {

		CAN_error_ID = CAN_Receive(&rx_buffer);
        if(CAN_error_ID != 0){
		    itoa(CAN_error_ID, str, 2);
		    DEBUG_Print(str);
        }
		uint8_t count;
		uint8_t data[1];

        if (nxtMsg < msTicks){
            nxtMsg += 1000;
            data[0] = 0xAA;
            CAN_Transmit(data, 0x600);
        }
		if ((count = Chip_UART_Read(LPC_USART, uart_rx_buf, UART_RX_BUFFER_SIZE)) != 0) {
			switch (uart_rx_buf[0]) {
				case 'a':
					DEBUG_Print("Sending CAN with ID: 0x600\r\n");
					data[0] = 0xAA;
					CAN_error_ID = CAN_Transmit(data, 0x600);
                    if(CAN_error_ID != 0){
					    itoa(CAN_error_ID, str, 2);
					    DEBUG_Print(str);
                    }
					break;
				default:
					DEBUG_Print("Invalid Command\r\n");
					break;
			}
		}
	}
}
