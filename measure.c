#include <xc.h>

#include "measure.h"
#include "mcc_generated_files/mcc.h"

#define VOLTAGE_CHANNEL 0x5
#define CURRENT_CHANNEL 0x6
#define ADC_RESOLUTION  (1024 - 1)
#define ADC_REFH        3300
#define GAIN            66
#define RESISTOR        3

// Number of samples to do the averaging during measures
#define AVERAGE_SAMPLES 8

void adc_init(void) {
	// TODO -> complete adc initialisation
    //offsetCurrent = measure_current(0);
}

/**
 * Read one ADC channel. This function is only
 * local to this file.
 * This function make the average on samples
 *
 * @param channel : the channel to be measured
 * @return the ADC read value with an average
 */
static uint16_t measure_adc(uint8_t channel) {
    uint32_t value = 0;
    
    // Make an average
    for(int i = 0; i < AVERAGE_SAMPLES; i++) {
        value += (uint16_t) (ADC_GetConversion(channel));
    }
    value /= AVERAGE_SAMPLES;
	return (uint16_t) (value);
}


/**
 * Measure voltage
 * @return 
 */
uint16_t measure_voltage() {
    uint32_t value = measure_adc(VOLTAGE_CHANNEL);
    
    // Convert sum from bits to mV
    value = (value * ADC_REFH) / ADC_RESOLUTION;
    return (uint16_t)(value);
}

/**
 * 
 * @param offset
 * @return 
 */
uint16_t measure_current(uint16_t offset) {
	uint32_t value = measure_adc(CURRENT_CHANNEL);
    
    // Convert from bits to uA
    value = (value * ADC_REFH) / ADC_RESOLUTION;    // [mV]
    value *= 1000;  // [uV]
    value /= GAIN;  // [uV]
    value /= RESISTOR;  // [uA]
    
    // Return value without offset or null if it's too low
    if(value > offset) return (uint16_t)(value-offset);
    return 0;
}