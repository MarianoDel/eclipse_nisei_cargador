/*
 * hard.c
 *
 *  Created on: 11/02/2016
 *      Author: Mariano
 */
#include "hard.h"
#include "stm32f0xx.h"


// ------- Externals de los timers -------
extern volatile unsigned short timer_led_error;

// ------- de los led y errores -------
enum var_error_states error_state = ERROR_NO;
unsigned char error_bips = 0;
unsigned char error_bips_counter = 0;


// ------- Funciones del Modulo -------
void UpdateErrors (void)
{
	switch (error_state)
	{
		case ERROR_NO:
			LEDR_OFF;
			break;

		case ERROR_IPEAK:
			error_bips = 1;
			error_state = ERROR_RUN;
			break;

		case ERROR_VIN:
			error_bips = 2;
			error_bips_counter = error_bips;
			error_state = ERROR_RUN;
			break;

		case ERROR_VBAT:
			error_bips = 3;
			error_bips_counter = error_bips;
			error_state = ERROR_RUN;
			break;

		case ERROR_TEMP:
			error_bips = 4;
			error_bips_counter = error_bips;
			error_state = ERROR_RUN;
			break;

		case ERROR_RUN:
			if (!timer_led_error)
			{
				error_state = ERROR_RUN_A;
				error_bips_counter = error_bips;
			}

			break;

		case ERROR_RUN_A:
			if (!timer_led_error)
			{
				if (error_bips_counter)
				{
					LEDR_ON;
					error_bips_counter--;
					error_state++;
					timer_led_error = TT_BIP_SHORT;
				}
				else
				{
					LEDR_OFF;
					timer_led_error = TT_BIP_LONG;
					error_state = ERROR_RUN;		//termina el ciclo hago la espera larga
				}
			}
			break;

		case ERROR_RUN_B:
			if (!timer_led_error)
			{
				LEDR_OFF;
				timer_led_error = TT_BIP_SHORT;
				error_state--;
			}
			break;

		default:
			LEDR_OFF;
			error_state = ERROR_NO;
			timer_led_error = 0;
			break;
	}
}

void ErrorCommands(unsigned char command)
{
	error_state = command;
}


