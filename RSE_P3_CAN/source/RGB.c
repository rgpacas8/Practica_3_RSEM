/*
 * RGB.c
 *
 *  Created on: 10 sep 2019
 *      Author: Ricardo Pacas
 */

#include "MK64F12.h"
#include "RGB.h"
#include <GPIO.h>
#include "bits.h"

#define ASCENDENTE_SW2  (0x02u) // Orden1 de FSM
#define DESCENDENTE_SW3 (0x01u)	// Orden2 de FSM

typedef enum {
	GREEN,		// 0
	BLUE,		// 1
	PURPLE,		// 2
	RED,		// 3
	YELLOW,		// 4
}State_name_t;

void RGB_init(void) {
	/**	Configurar el Clock Gating de los perifericos GPIO a utilizar */

	GPIO_clock_gating(GPIO_E);	// led verde
	gpio_pin_control_register_t led_config = GPIO_MUX1;	// 100 de GPIO
	/** OUTPUT: Configurar como LED RGB (GPIO) + safe value + como output */

	 // ledRed   - pin 22 GPIO
	GPIO_pin_control_register(GPIO_E, bit_26, &led_config); // ledGreen - pin 26 GPIO


	GPIO_set_pin(GPIO_E, bit_26);	// OFF - 1 GREEN

	GPIO_data_direction_pin(GPIO_E, GPIO_OUTPUT, bit_26); // OUTPUT - 1 GREEN

	gpio_pin_control_register_t pin_control_register_bit_d_0 = GPIO_MUX1;
	GPIO_clock_gating(GPIO_E);	 						   // Pin 0 del puerto D
	GPIO_pin_control_register(GPIO_E, bit_24, &pin_control_register_bit_d_0);// GPIO


	GPIO_data_direction_pin(GPIO_E, GPIO_OUTPUT, bit_24);


	GPIO_clear_pin(GPIO_E, bit_24);	// OFF -  Motor

}

void sw_init(void){
	GPIO_clock_gating( GPIO_A);	// sw3
	GPIO_clock_gating( GPIO_C);	// sw2

	gpio_pin_control_register_t sw_config  = GPIO_MUX1|GPIO_PE|GPIO_PS|INTR_FALLING_EDGE;	// 103
	GPIO_pin_control_register( GPIO_A, bit_4,  &sw_config );  // sw3	  - pin  4
	GPIO_pin_control_register( GPIO_C, bit_6,  &sw_config );  // sw2	  - pin  6

	GPIO_data_direction_pin(GPIO_A,GPIO_INPUT, bit_4);
	GPIO_data_direction_pin(GPIO_C,GPIO_INPUT, bit_6);
}

uint8_t encender_LED(color_ON color_RGB) {
	switch (color_RGB) {
	case RED_ON:
		GPIO_clear_pin(GPIO_B, bit_22);	// ON - 0 RED
		break;
	case GREEN_ON:
		GPIO_clear_pin(GPIO_E, bit_26);	// ON - 0 GREEN
		break;
	case BLUE_ON:
		GPIO_clear_pin(GPIO_B, bit_21);	// ON - 0 BLUE
		break;
	case PURPLE_ON:
		GPIO_clear_pin(GPIO_B, bit_21);	// ON - 0 	BLUE +
		GPIO_clear_pin(GPIO_B, bit_22);	// ON - 0 	RED		= PURPLE
		break;
	case YELLOW_ON:
		GPIO_clear_pin(GPIO_B, bit_22);	// ON - 0 	RED +
		GPIO_clear_pin(GPIO_E, bit_26);	// ON - 0 	GREEN	= YELLOW
		break;
	default: /**If doesn't exist the option*/
		return (FALSE);
	}
	return (TRUE);
}

uint8_t apagar_LED(color_OFF color_RGB) {
	switch (color_RGB) {
	case RED_OFF:
		GPIO_set_pin( GPIO_B, bit_22);	// OFF - 1 RED
		break;
	case GREEN_OFF:
		GPIO_set_pin( GPIO_E, bit_26);	// OFF - 1 GREEN
		break;
	case BLUE_OFF:
		GPIO_set_pin( GPIO_B, bit_21);	// OFF - 1 BLUE
		break;
	case RGB_OFF:
		GPIO_set_pin( GPIO_B, bit_22);	// OFF - 1 RED	  +
		GPIO_set_pin( GPIO_E, bit_26);	// OFF - 1 GREEN  +
		GPIO_set_pin( GPIO_B, bit_21);	// OFF - 1 BLUE		=	RGB is OFF
		break;
	default: /**If doesn't exist the option*/
		return (FALSE);
	}
	return (TRUE);
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
void delay(uint16_t delay) {
	volatile uint16_t j, i;

	for (j = delay; j > 0; j--) {
		for (i = 0; i < 1000; ++i) {
			__asm("nop");
		}

	}
}
