/*
 * ADC.c
 *
 *  Created on: 12 mar. 2021
 *      Author: RICARDO
 */


#include "MK64F12.h"
#include "fsl_adc16.h"
#include "ADC.h"
#include <stdio.h>

const uint32_t g_Adc16_12bitFullRange = 4096U;

adc16_config_t adc16ConfigStruct;
adc16_channel_config_t adc16ChannelConfigStruct;

void ADC0_init_demo(void){

ADC16_GetDefaultConfig(&adc16ConfigStruct);
ADC16_Init(DEMO_ADC16_BASE, &adc16ConfigStruct);
ADC16_EnableHardwareTrigger(DEMO_ADC16_BASE, false);

adc16ChannelConfigStruct.channelNumber                        = DEMO_ADC16_USER_CHANNEL;
adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false;
}

uint16_t ADC_value_demo(void){
	ADC16_SetChannelConfig(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
	//PRINTF("ADC Value: %d\r\n", ADC16_GetChannelConversionValue(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP));
	return (ADC16_GetChannelConversionValue(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP));
}
