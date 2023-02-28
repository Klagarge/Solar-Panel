#include <xc.h>

#include "measure.h"
#include "mcc_generated_files/mcc.h"

#define VOLTAGE_CHANNEL 0x5
#define CURRENT_CHANNEL 0x6
#define ADC_RESOLUTION  4096 - 1
#define ADC_REFH        3300
#define GAIN            66
#define RESISTOR        3

// Number of samples to do the averaging during measures
#define AVERAGE_SAMPLES 8

uint16_t samplesVoltage[AVERAGE_SAMPLES];
uint16_t samplesCurrent[AVERAGE_SAMPLES];


void adc_init(void)
{
	// TODO -> complete adc initialisation
    offsetCurrent = measure_current(0);

}

/**
 * Read one ADC channel. This function is only
 * local to this file.
 *
 * @param channel : the channel to be measured
 * @return the ADC read value
 */
static uint16_t measure_adc(uint8_t channel)
{
	return (uint16_t) (ADC_GetConversion(channel));
}

uint16_t measure_voltage()
{
	uint16_t m = measure_adc(VOLTAGE_CHANNEL);
    m /= 20; // TODO Explain why 20
    return m;
}

uint16_t measure_current(uint16_t offset)
{
	uint16_t m = measure_adc(CURRENT_CHANNEL);
    m -= offset;
    if(m <= 0){
        m = 0;
    }
    m /= GAIN;
    return m;
}