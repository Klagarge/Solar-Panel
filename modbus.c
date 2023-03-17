/*!
 * @file modbus.c
 * @authors Simon Donnet-Monay & Remi Heredero
 * @date 14 march 2023
 * @brief 
 */

#include "modbus.h"
#include "crc.h"

// Modbus functions
#define READ_INPUT_REGISTERS    0x04 // Modbus function for read input register
#define READ_HOLDING_REGISTERS  0x03 // Modbus function for read holding register
#define WRITE_SINGLE_REGISTER   0x06 // Modbus function for write a single register

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

/**
 * End of MODBUS frame. 
 */
void modbus_timer(void) {
	INTCONbits.TMR0IF = 0;          // Reset flag of the timer0 interrupt 
    recPtr = 0;                     // Reset position of the char in the frame
    TMR0_StopTimer();               // Stop timer who detect the end of the frame
    modbus_analyse_and_answer();    // Run analyse of this frame
}

extern uint16_t measure_voltage();

/**
 * @brief Analyse the received frame and build an answer
 * @return The error code if the frame isn't valid (TODO)
 */
uint8_t modbus_analyse_and_answer(void) {
    // Init lenght of the answer frame at 0
    uint16_t length = 0;
    
    // Check if the received frame is for this device
    if(rx_buf[0] == modbusAddress){
        tx_buf[0] = rx_buf[0];  // Copy the address on the tx buffer
        tx_buf[1] = rx_buf[1];  // Copy the function number on the tx buffer
        
        // Init the addresse Register local variable
        uint16_t adresseRegister = ((uint16_t)rx_buf[2] << 8) | rx_buf[3];
        
        switch(rx_buf[1]){  // Check the function from rx buffer
            
            // In case of the function is to read input register: 
            case READ_INPUT_REGISTERS: 
                // Define length as the number of register we want read
                length = ((uint16_t)rx_buf[4] << 8) | rx_buf[5];
                
                // Write this length on the tx buffer for the answer
                tx_buf[2] = (uint8_t)(length*2);
                
                // For each register, write the value on the tx buffer (register on 16bits)
                for(uint16_t i = 0; i < length; i++){   // Data
                    tx_buf[i*2+4] = input_registers[adresseRegister+i];
                    tx_buf[i*2+3] = (input_registers[adresseRegister+i] >> 8);
                }
                
                // Transform length as the number of bytes on tx register
                length*=2;  // 2 bytes by register
                length+=3;  // + address + function + length
                break;
            
                
            // In case of the function is to read holding register
            case READ_HOLDING_REGISTERS:
                // Define length as the number of register we want read
                length = ((uint16_t)rx_buf[4] << 8) | rx_buf[5];
                
                // Write this length on the tx buffer for the answer
                tx_buf[2] = (uint8_t)(length*2);
                
                // For each register, write the value on the tx buffer (register on 16bits))
                for(uint16_t i = 0; i < length; i++){
                    tx_buf[i*2+4] = holding_registers[adresseRegister+i];
                    tx_buf[i*2+3] = (holding_registers[adresseRegister+i] >> 8);
                }
                // Transform length as the number of bytes on tx register
                length*=2;  // 2 bytes by register
                length+=3;  // + address + function + length
                break;
                
            
            // In case of the funciton is to write a single register
            case WRITE_SINGLE_REGISTER:
                
                // Write the value on rx buffer on the holding register define by the adress register we define before
                holding_registers[adresseRegister] = ((uint16_t)rx_buf[4] << 8) | rx_buf[5];
                
                // Copy data on the tx buffer
                for (uint8_t i = 2; i <= 5; i++) {
                    tx_buf[i] = rx_buf[i];
                    length = i+1;
                }
                break;
        }
        
    }
      
    // Clear address on rx buffer (for validate we treat the data)
    rx_buf[0] = 0;
    
    // Send the answer frame
    modbus_send(length);
    
    // TODO return error code
  
}

/**
 * Record a char when it's received on the modbus
 */
void modbus_char_recvd(void) {
    //! Record the received char on the rx buffer and move position of the record pointer for the next char
	rx_buf[recPtr++] = RCREG1;
    
    //! Reload and start the timer0 for restart to count the time between char
    TMR0_Reload();
    TMR0_StartTimer();
}

/**
 * Create the CRC and send the tx buffer
 * @param length lenght of the frame without the CRC
 */
void modbus_send(uint8_t length) {
    // Create the CRC
	uint16_t crc = CRC16(tx_buf, length);
	
    // Write CRC on the tx buffer
    tx_buf[length] = crc;
    tx_buf[length+1] = crc >> 8;

	length += 2; //! add 2 CRC bytes for total size

	// Send each byte of the frame on the tx buffer
    for (uint8_t i = 0; i < length; i++){
          EUSART1_Write(tx_buf[i]);
      }
}

/**
 * Initialize the modbus with adress and handler function
 * @param address The adress of this device on modbus protocole
 */
void modbus_init(uint8_t address) {
	// Init the modbus adress
    modbusAddress = address;
    
    // Save the modbus adress in the dedicated register
    holding_registers[1] = address;
    
    // Set the handler for the character detection
    EUSART1_SetRxInterruptHandler(modbus_char_recvd);
    
    // Set the handler for the detection of end frame
    TMR0_SetInterruptHandler(modbus_timer);
}