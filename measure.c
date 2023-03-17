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

void adc_init(void)
{
	// TODO -> complete adc initialisation
    //offsetCurrent = measure_current(0);

}

/**
 * Read one ADC channel. This function is only
 * local to this file.
 *
 * @param channel : the channel to be measured
 * @return the ADC read value
 */
static uint16_t measure_adc(uint8_t channel) {
	return (uint16_t) (ADC_GetConversion(channel));
}


/**
 * 
 * @return 
 */
uint16_t measure_voltage() {
    uint32_t sum = 0;
    
    // Make an average
    for(int i = 0; i < AVERAGE_SAMPLES; i++) {
        sum += measure_adc(VOLTAGE_CHANNEL);
    }
    sum /= AVERAGE_SAMPLES;
    
    // Convert sum from bits to mV
    sum = (sum * ADC_REFH) / ADC_RESOLUTION;
    return (uint16_t)(sum);
}

uint16_t measure_current(uint16_t offset) {
	uint32_t sum = 0;
    for(int i = 0; i< AVERAGE_SAMPLES; i++){
        sum += measure_adc(CURRENT_CHANNEL);
    } 
    uint32_t m = (sum / AVERAGE_SAMPLES);   // m is bits
    m = (m * ADC_REFH) / ADC_RESOLUTION;    // m is mV
    m *= 1000;                              // m is uV
    m /= GAIN;
    m /= RESISTOR;                          // m is uA
    if(m <= offset){
        m = 0;
    } else {
        m -= offset;
    }
    
    return (uint16_t)m;
}