#include "ch.h"
#include "hal.h"
#include "sensors/adc.h"
#include <stdlib.h>

#define ADC_NUM_CHANNELS	4		/* solar, battery, temperature */
#define VCC_REF				3300	/* mV */

#define DIVIDER_VIN		10	/* VBat -- 90KOhm -- ADC -- 10kOhm -- GND */

/**
 * @brief	Called when ADC finishes converting; does nothing at the moment
 */
void adccb(ADCDriver *adcp, adcsample_t *buffer, size_t n)
{
	(void)adcp;
	(void)buffer;
	(void)n;
}

/*
 * ADC conversion group.
 * Mode:        Linear buffer, 4 samples of 4 channels, SW triggered.
 * Channels:    Solar voltage divider    ADC1_IN12
 *              USB voltage divider      ADC1_IN14
 *              Battery voltage divider  ADC1_IN9
 *              Temperature sensor       ADC1_IN16
 */
static const ADCConversionGroup adcgrpcfg = {
	FALSE,
	ADC_NUM_CHANNELS,
	adccb,
	NULL,
	/* HW dependent part.*/
	0,
	ADC_CR2_SWSTART,
	ADC_SMPR1_SMP_AN14(ADC_SAMPLE_144)
		| ADC_SMPR1_SMP_AN12(ADC_SAMPLE_144)
		| ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_144),
	ADC_SMPR2_SMP_AN9(ADC_SAMPLE_144),
	ADC_SQR1_NUM_CH(ADC_NUM_CHANNELS),
	0,
	ADC_SQR3_SQ1_N(ADC_CHANNEL_IN12)
		| ADC_SQR3_SQ2_N(ADC_CHANNEL_IN14)
		| ADC_SQR3_SQ3_N(ADC_CHANNEL_IN9)
		| ADC_SQR3_SQ4_N(ADC_CHANNEL_SENSOR)
};

/*
 * @brief	Initializes and configures the Analog to Digital Converter (ADC) 
 * @note	Enables the ADC clock, making it ready for conversion
 *
 * @return	The error state (only returns SENSOR_OK at the moment)
 * @see		SensorErr
 */
enum SensorErr adc_init(void)
{
	adcStart(&ADCD1, NULL);
	adcSTM32EnableTSVREFE();
	palSetLineMode(LINE_ADC_VIN, PAL_MODE_INPUT_ANALOG); // Vin
	return SENSOR_OK;
}

/*
 * @brief	Deactivates the ADC peripheral	
 * @note	Puts the ADC into low power mode	
 * 
 * @return	The error state (only returns SENSOR_OK at the moment)
 * @see		SensorErr
 */
enum SensorErr adc_shutdown(void) {
	// TODO Error handling?
	adcStop(&ADCD1);
	return SENSOR_OK;
}
 
/*
 * @brief	Gets data from the ADC on the MCU
 *
 * @pre		@p adc_init() has been called
 * @return	A structure containing battery voltage, temperature, and an error state
 * @see		adc_t
 */
struct adc_t adc_get(void)
{
	adcsample_t samples[ADC_NUM_CHANNELS]; // ADC sample buffer
	 
	adc_init();
	adcStartConversion(&ADCD1, &adcgrpcfg, samples, 1);
	chThdSleep(TIME_MS2I(50)); // FIXME naive wait until conversion is finished
	adc_shutdown();
	// FIXME we have no way of checking if these are good values; they could be
	// random spots in memory.
	struct adc_t data = adc_t_init();
	data.vbat = samples[1] * VCC_REF * DIVIDER_VIN / 4096;
	data.therm = (((int32_t)samples[3] * 40 * VCC_REF / 4096) - 30400) + 2500;
	return data;
}
