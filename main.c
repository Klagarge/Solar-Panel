/*!
 * @file main.c
 * @authors Simon Donnet-Monay & Remi Heredero
 * @date 14 march 2023
 * @brief Main is in two part.
 * First for setup everything
 * Second for loop on measure and set duty cycle for PWM
 */


#include "mcc_generated_files/mcc.h"
#include "lcd/lcd.h"
#include "measure.h"
#include "modbus.h"

#define MAX_COL 16 //!< columns for lcd screen (and 2 rows)

void main(void) {
    
/* SETUP FUNCTIONS: */
    
    // Initialize the system with all mcc default configs
    SYSTEM_Initialize();
    
    Lcd_Init();         // Initialize lcd screen
    adc_init();         // Initialize adc for measures
    modbus_init(0x80);  // Initialize all specific modbus function

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();
    
    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();
    
    /*
     * Initialize offset current.
     * 1. disable load
     * 2. Measure current without load
     * 
     * The goal it's to remove the offset due to the electronics parts
     */
    EPWM1_LoadDutyValue(0);
    const uint16_t offsetCurrent = measure_current(offsetCurrent);
    
    // create a char array for display on lcd (with space for '\0')
    char msg[MAX_COL+1]; 
    
/* LOOP MAIN PROGRAM: */
    
    while (1) {
        
        // Get the measure and save it and the appropriate register
        input_registers[0] = measure_voltage();
        input_registers[1] = measure_current(offsetCurrent);
        
        // Print on the first row of the lcd the Voltage
        sprintf(msg, "U = %04d [mV] ", input_registers[0]);
        LCD_2x16_WriteMsg(msg,0);
        
        // Print on the second row of the lcd the current
        sprintf(msg, "I = %04d [uA] ", input_registers[1]);
        LCD_2x16_WriteMsg(msg,1);
        
        // Write the duty cycle for pwm from the appropriate register
        EPWM1_LoadDutyValue(holding_registers[0]);
    }   
}