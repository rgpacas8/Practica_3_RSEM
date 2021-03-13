/*
 * ADC.h
 *
 *  Created on: 12 mar. 2021
 *      Author: RICARDO
 */

#ifndef ADC_H_
#define ADC_H_

#include "stdint.h"
#include "MK64F12.h"

#define DEMO_ADC16_BASE          ADC0
#define DEMO_ADC16_CHANNEL_GROUP 0U
#define DEMO_ADC16_USER_CHANNEL  12U

void ADC0_init_demo(void);
uint16_t ADC_value_demo(void);

#endif /* ADC_H_ */
