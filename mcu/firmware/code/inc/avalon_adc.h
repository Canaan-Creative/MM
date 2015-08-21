/*
 * @brief adc head file
 *
 * @note
 *
 * @par
 */
#ifndef __AVALON_ADC_H_
#define __AVALON_ADC_H_

#include "adc_11xx.h"

#define ADC_RNTCP1	(ADC_CH6)
#define ADC_RNTCP2	(ADC_CH0)
#define ADC_RNTCP3	(ADC_CH1)
#define ADC_RNTCP4	(ADC_CH2)
#define ADC_VCC12VIN	(ADC_CH3)
#define ADC_VCC3V3	(ADC_CH7)

#define ADC_CAPCOUNT	6

void adc_init(void);
void adc_read(uint8_t channel, uint16_t *data);

#endif /* __AVALON_ADC_H_ */
