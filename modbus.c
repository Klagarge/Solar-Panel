#include "modbus.h"
#include "crc.h"
#include "uart.h"
#include <xc.h>
#include <stdint.h>
#include <stdio.h>

// Modbus functions
#define READ_INPUT_REGISTERS    0x04
#define READ_HOLDING_REGISTERS  0x03
#define WRITE_SINGLE_REGISTER   0x06

// Modbus data model
uint8_t modbusAddress;
uint16_t input_registers[2];
uint16_t holding_registers[2];


// Modbus error codes
#define ILLEGAL_FUNCTION		1
#define ILLEGAL_DATA_ADDRESS	2
#define ILLEGAL_DATA_VALUE		3
#define SLAVE_DEVICE_FAILURE	4

/**
 * Buffers for serial receive and send operations 
 * which are more than one byte long
 **/
uint8_t rx_buf[256];
uint8_t tx_buf[256];

// Current position pointer for storing receive position
uint8_t recPtr = 0;

void modbus_timer(void)
{
	INTCONbits.TMR0IF = 0;
    recPtr = 0;   
    TMR0_StopTimer();
    modbus_analyse_and_answer();
   
}

uint8_t modbus_analyse_and_answer(void) {
	// TODO -> complete the modbus analyse and answer
    rx_buf[0] = 0;
    
   // sprintf(tx_buf, "%i", modbusAddress, );
  
}

void modbus_char_recvd(void)
{
	rx_buf[recPtr++] = RCREG1;
    TMR0_Reload();
    TMR0_StartTimer();
}

void modbus_send(uint8_t length)
{
	uint16_t temp16; 
	uint8_t i;

	// TODO -> complete modbus RCR calculation
	length += 2; // add 2 CRC bytes for total size

	// For all the bytes to be transmitted
  uart_send(tx_buf,length);
}

void modbus_init(uint8_t address)
{
	modbusAddress = address;
    EUSART1_SetRxInterruptHandler(modbus_char_recvd);
    TMR0_SetInterruptHandler(modbus_timer);
}
