#include "uart.h"

void uart_send(uint8_t *tx_buf, uint8_t length){
    for (uint8_t i = 0; i < length; i++){
        EUSART1_Write(tx_buf[i]);
    }
}
