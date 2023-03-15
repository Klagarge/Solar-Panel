/*!
 * @file modbus.c
 * @authors Simon Donnet-Monay & Remi Heredero
 * @date 14 march 2023
 * @biref
 */

#include "modbus.h"
#include "crc.h"

// Modbus functions
#define READ_INPUT_REGISTERS    0x04 //!< Modbus function for read input register = 04
#define READ_HOLDING_REGISTERS  0x03 //!< Modbus function for read holding register = 03
#define WRITE_SINGLE_REGISTER   0x06 //!< Modbus function for write a single register = 06

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

void modbus_timer(void) {
	INTCONbits.TMR0IF = 0;
    recPtr = 0;   
    TMR0_StopTimer();
    modbus_analyse_and_answer();
}
extern uint16_t measure_voltage();
uint8_t modbus_analyse_and_answer(void) {
	// TODO -> complete the modbus analyse and answer
    uint16_t length = 0;
    if(rx_buf[0] == modbusAddress){
        tx_buf[0] = rx_buf[0]; // Adress
        tx_buf[1] = rx_buf[1]; // Function
        uint16_t adresseRegister = ((uint16_t)rx_buf[2] << 8) | rx_buf[3];
        
        switch(rx_buf[1]){  // Check the function from rx buffer
            case READ_INPUT_REGISTERS:
                length = ((uint16_t)rx_buf[4] << 8) | rx_buf[5];
                tx_buf[2] = (uint8_t)(length*2);        // Data length
                for(uint16_t i = 0; i < length; i++){   // Data
                    tx_buf[i*2+4] = input_registers[adresseRegister+i];
                    tx_buf[i*2+3] = (input_registers[adresseRegister+i] >> 8);
                }
                length*=2;
                length+=3;
                break;
            case READ_HOLDING_REGISTERS:
                length = ((uint16_t)rx_buf[4] << 8) | rx_buf[5];
                tx_buf[2] = (uint8_t)(length*2);        // Data length
                for(uint16_t i = 0; i < length; i++){   // Data
                    tx_buf[i*2+4] = holding_registers[adresseRegister+i];
                    tx_buf[i*2+3] = (holding_registers[adresseRegister+i] >> 8);
                }
                length*=2;
                length+=3;
                break;
            case WRITE_SINGLE_REGISTER:
                holding_registers[adresseRegister] = ((uint16_t)rx_buf[4] << 8) | rx_buf[5];
                for (int i = 2; i <= 5; i++) {
                    tx_buf[i] = rx_buf[i];
                    length = i+1;
                }
                break;
        }
        
    }
       
    rx_buf[0] = 0;
    modbus_send(length);
  
}

void modbus_char_recvd(void)
{
	rx_buf[recPtr++] = RCREG1;
    TMR0_Reload();
    TMR0_StartTimer();
}

void modbus_send(uint8_t length)
{
    
	uint16_t crc = CRC16(tx_buf, length);
	
    tx_buf[length] = crc;
    tx_buf[length+1] = crc >> 8;

	length += 2; // add 2 CRC bytes for total size

	// For all the bytes to be transmitted
    for (uint8_t i = 0; i < length; i++){
          EUSART1_Write(tx_buf[i]);
      }
}

void modbus_init(uint8_t address)
{
	modbusAddress = address;
    holding_registers[1] = address;
    EUSART1_SetRxInterruptHandler(modbus_char_recvd);
    TMR0_SetInterruptHandler(modbus_timer);
}
