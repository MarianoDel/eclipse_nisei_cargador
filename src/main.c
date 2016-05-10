/**
  ******************************************************************************
  * @file    Template_2/main.c
  * @author  Nahuel
  * @version V1.0
  * @date    22-August-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * Use this template for new projects with stm32f0xx family.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"

//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>


//--- My includes ---//
#include "gpio.h"
#include "stm32f0x_tim.h"
#include "dsp.h"
#include "stm32f0xx_dma.h"


#include "hard.h"
//#include "main.h"



#include "core_cm0.h"
#include "adc.h"
#include "flash_program.h"


//--- VARIABLES EXTERNAS ---//
volatile unsigned char timer_1seg = 0;

volatile unsigned short timer_led_comm = 0;

// ------- Externals del Adc -------
volatile unsigned short adc_ch [5];
volatile unsigned char need_to_sync;
volatile unsigned char freq_ready;
volatile unsigned short lastC0V;
volatile unsigned char zero_cross = 0;

volatile unsigned char seq_ready = 0;

#define VIN			adc_ch[0]
#define VBAT		adc_ch[1]
#define IPEAK		adc_ch[2]
#define TEMP		adc_ch[3]
#define VSETLOAD	adc_ch[4]


// ------- Externals de los Timers -------
volatile unsigned short wait_ms_var = 0;
volatile unsigned short prog_timer = 0;
volatile unsigned short timer_led_error = 0;

// ------- Externals de los modos -------




//--- VARIABLES GLOBALES ---//


// ------- de los timers -------
volatile unsigned short timer_standby;
volatile unsigned char filter_timer;
volatile unsigned short secs = 0;
volatile unsigned short msecs = 0;



//volatile unsigned char door_filter;
//volatile unsigned char take_sample;
//volatile unsigned char move_relay;
//volatile unsigned char secs = 0;
//volatile unsigned short minutes = 0;


// ------- de los filtros de mediciones -------
#define LARGO_F		32
#define DIVISOR_F	5
unsigned char vd0 [LARGO_F + 1];
unsigned char vd1 [LARGO_F + 1];
unsigned char vd2 [LARGO_F + 1];
unsigned char vd3 [LARGO_F + 1];
unsigned char vd4 [LARGO_F + 1];


#define RCC_DMA_CLK (RCC->AHBENR & RCC_AHBENR_DMAEN)
#define RCC_DMA_CLK_ON 		RCC->AHBENR |= RCC_AHBENR_DMAEN
#define RCC_DMA_CLK_OFF 	RCC->AHBENR &= ~RCC_AHBENR_DMAEN

//--- FILTROS DE SENSORES ---//
#define LARGO_FILTRO 16
#define DIVISOR      4   //2 elevado al divisor = largo filtro
//#define LARGO_FILTRO 32
//#define DIVISOR      5   //2 elevado al divisor = largo filtro
unsigned short vtemp [LARGO_FILTRO + 1];
unsigned short vpote [LARGO_FILTRO + 1];

//--- FIN DEFINICIONES DE FILTRO ---//


//--- FUNCIONES DEL MODULO ---//
void DMAConfig(void);
unsigned char MAFilter (unsigned char, unsigned char *);
unsigned char CheckPolarity (void);
unsigned char CheckVoltageMin (void);


//-------------------------------------------//
// @brief  Main program.
// @param  None
// @retval None
//------------------------------------------//
int main(void)
{
	unsigned char i;
	unsigned char onsync = 0;
	enum var_main_states main_state = MAIN_STANDBY;

	//!< At this stage the microcontroller clock setting is already configured,
    //   this is done through SystemInit() function which is called from startup
    //   file (startup_stm32f0xx.s) before to branch to application main.
    //   To reconfigure the default setting of SystemInit() function, refer to
    //   system_stm32f0xx.c file

	//GPIO Configuration.
	GPIO_Config();


	//ACTIVAR SYSTICK TIMER
	if (SysTick_Config(48000))
	{
		while (1)	/* Capture error */
		{
			if (LEDR)
				LEDR_OFF;
			else
				LEDR_ON;

			for (i = 0; i < 255; i++)
			{
				asm (	"nop \n\t"
						"nop \n\t"
						"nop \n\t" );
			}
		}
	}


	 //PRUEBA LEDS
	/*
	 while (1)
	 {
		 if (LEDG)
		 {
			 LEDG_OFF;
			 LEDY_OFF;
			 LEDR_OFF;
		 }
		 else
		 {
			 LEDG_ON;
			 LEDY_ON;
			 LEDR_ON;
		 }

		 Wait_ms(150);
	 }
	*/
	 //FIN PRUEBA LEDS

	//PRUEBA SYNC
	/*
	while (1)
	{
		if (LEDG)
		{
			LEDG_OFF;
			SYNC_OFF;
		}
		else
		{
			LEDG_ON;
			SYNC_ON;
		}

		Wait_ms(1000);
	}
	*/
	//FIN PRUEBA SYNC

	//PRUEBA ERRORES
	/*
	onsync = 0;
	while (1)
	{
		switch (onsync)
		{
			case 0:
				if (!timer_standby)
				{
					onsync++;
					ErrorCommands(ERROR_TEMP);
					timer_standby = 10000;		//10 segs
					LEDG_ON;
				}
				break;

			case 1:
				if (!timer_standby)
				{
					onsync++;
					ErrorCommands(ERROR_NO);
					timer_standby = 5000;		//5 segs
					LEDG_OFF;
				}
				break;

			case 2:
				if (!timer_standby)
				{
					onsync = 0;
				}
				break;

			default:
				onsync = 0;
				break;
		}
		UpdateErrors();
	}
	*/
	//FIN PRUEBA ERRORES

	//TIM Configuration.
	TIM_1_Init();
	TIM_14_Init();
	//Timer_3_Init();
	//Timer_4_Init();

	LEDG_ON;
	Wait_ms(1000);
	LEDG_OFF;

	//ADC configuration.
	AdcConfig();
	ADC1->CR |= ADC_CR_ADSTART;

	//DMA configuration.
	DMAConfig();


	//--- Prueba ADC y DMA ---//

	while(1)
	{
		//busco sync con DMA
		if ((!onsync) && (ADC1->ISR & ADC_IT_EOC))
		{
			ADC1->ISR |= ADC_IT_EOC;
			onsync = 1;
			DMA1_Channel1->CCR |= DMA_CCR_EN;
			LEDG_ON;
		}

		//me fijo si hubo overrun
		if (ADC1->ISR & ADC_IT_OVR)
		{
			ADC1->ISR |= ADC_IT_EOC | ADC_IT_EOSEQ | ADC_IT_OVR;
			if (LEDY)
				LEDY_OFF;
			else
				LEDY_ON;
		}

		if (DMA1->ISR & DMA1_FLAG_TC1)
		{
		    // Clear DMA TC flag
			DMA1->IFCR = DMA1_FLAG_TC1;

			switch (main_state)
			{
				case MAIN_STANDBY:
					//si tengo bateria conectada y necesita carga, lo hago
					if ((CheckVBAT() > VBAT_MIN_SET) && (CheckVBAT() < VSETLOAD))
					{
						//empiezo a cargar siempre en SYNC
						if (onsync)
						{
							main_state = MAIN_CHARGING;
							MOSFET_ON;
							LEDY_ON;
						}
					}
					break;

				case MAIN_CHARGING:
					//reviso no pasarme de corriente si estoy con el mosfet activo
					if ((IPEAK > PEAK_CURRENT_SET) && (MOSFET))
					{
						MOSFET_OFF;
						LEDY_OFF;
						ErrorCommands(ERROR_IPEAK);
						main_state = MAIN_ERROR_IPEAK;
						break;
					}

					if (CheckVBAT() > VSETLOAD)		//cuento 60 segs y vuelvo a standby
					{
						MOSFET_OFF;
						secs++;

						if (secs > TT_CHARGING)
							main_state = MAIN_STANDBY;
					}
					else
					{
						//prendo mosfet siempre onsync
						if (onsync)
							MOSFET_ON;

						if (secs)
							secs--;
					}

					if (CheckVBAT() > VBAT_MIN_SET)
					{
						MOSFET_OFF;
						LEDY_OFF;
						ErrorCommands(ERROR_VBAT);
						main_state = MAIN_ERROR_VBAT;
						timer_standby = TT_ERROR_VBAT;
					}

					break;

				case MAIN_ERROR_IPEAK:
					//a diferencia de otros casos reestablezco al toque el error
					ErrorCommands(ERROR_NO);
					main_state = MAIN_STANDBY;
					break;

				case MAIN_ERROR_VIN:
					break;

				case MAIN_ERROR_VBAT:
					if (!timer_standby)
					{
						if (CheckPolarity() == RESP_NO)
						{
							ErrorCommands(ERROR_NO);
							main_state = MAIN_STANDBY;
						}
						else
							timer_standby = TT_ERROR_VBAT;
					}
					break;

				case MAIN_ERROR_TEMP:
					break;

				default:
					main_state = MAIN_STANDBY;
					break;
			}

			//----- Verificaciones comunes a todos los casos -----//
			//verifico polaridad y tension minima
			if (main_state < MAIN_ERROR_IPEAK)
			{
				if ((CheckPolarity() == RESP_YES) || (CheckVoltageMin() == RESP_YES))
				{
					MOSFET_OFF;
					main_state = MAIN_ERROR_VBAT;
					ErrorCommands(ERROR_VBAT);
					timer_standby = TT_ERROR_VBAT;
				}
			}

			//Verifico fase con ADC
			if (VIN > VOLTAGE_SYNC_ON)
			{
				if ((!onsync) && (!SYNC))
					onsync = 1;
				else
					onsync = 0;

				SYNC_ON;
			}
			else if (VIN < VOLTAGE_SYNC_OFF)
				SYNC_OFF;


		}

		//Resuelvo cuestiones que no tengan que ver con las muestras

		//muestro el led de error segun error_state
		UpdateErrors();

	}

	//--- Fin Prueba ADC y DMA ---//
	return 0;
}
//--- End of Main ---//


unsigned char CheckPolarity (void)
{
	//reviso el ultimo valor de VBAT
	if (VBAT < VOLTAGE_ZERO)	//debe ser tension negativa en la bateria
		return RESP_YES;
	else
		return RESP_NO;
}

unsigned char CheckVoltageMin (void)
{
	if (VBAT < VOLTAGE_MIN)
		return RESP_YES;
	else
		return RESP_NO;
}

/*
unsigned short Get_Temp (void)
{
	unsigned short total_ma;
	unsigned char j;

	//Kernel mejorado ver 2
	//si el vector es de 0 a 7 (+1) sumo todas las posiciones entre 1 y 8, acomodo el nuevo vector entre 0 y 7
	total_ma = 0;
	vtemp[LARGO_FILTRO] = ReadADC1 (CH_IN_TEMP);
    for (j = 0; j < (LARGO_FILTRO); j++)
    {
    	total_ma += vtemp[j + 1];
    	vtemp[j] = vtemp[j + 1];
    }

    return total_ma >> DIVISOR;
}
*/

unsigned char MAFilter (unsigned char new_sample, unsigned char * vsample)
{
	unsigned short total_ma;
	unsigned char j;

	//Kernel mejorado ver 2
	//si el vector es de 0 a 7 (+1) sumo todas las posiciones entre 1 y 8, acomodo el nuevo vector entre 0 y 7
	total_ma = 0;
	*(vsample + LARGO_F) = new_sample;

    for (j = 0; j < (LARGO_F); j++)
    {
    	total_ma += *(vsample + j + 1);
    	*(vsample + j) = *(vsample + j + 1);
    }

    return total_ma >> DIVISOR_F;
}

unsigned short MAFilter16 (unsigned char new_sample, unsigned char * vsample)
{
	unsigned short total_ma;
	unsigned char j;

	//Kernel mejorado ver 2
	//si el vector es de 0 a 7 (+1) sumo todas las posiciones entre 1 y 8, acomodo el nuevo vector entre 0 y 7
	total_ma = 0;
	*(vsample + LARGO_F) = new_sample;

    for (j = 0; j < (LARGO_F); j++)
    {
    	total_ma += *(vsample + j + 1);
    	*(vsample + j) = *(vsample + j + 1);
    }

    return total_ma >> DIVISOR_F;
}


void TimingDelay_Decrement(void)
{
	if (wait_ms_var)
		wait_ms_var--;

	if (timer_standby)
		timer_standby--;

	if (filter_timer)
		filter_timer--;

	if (timer_led_error)
		timer_led_error--;

	if (msecs < 1000)
		msecs++;
	else
	{
		msecs = 0;
		secs++;
	}
}


void DMAConfig(void)
{
	/* DMA1 clock enable */
	if (!RCC_DMA_CLK)
		RCC_DMA_CLK_ON;

	//Configuro el control del DMA CH1
	DMA1_Channel1->CCR = 0;
	DMA1_Channel1->CCR |= DMA_Priority_VeryHigh | DMA_MemoryDataSize_HalfWord | DMA_PeripheralDataSize_HalfWord | DMA_MemoryInc_Enable;
	//DMA1_Channel1->CCR |= DMA_Mode_Circular | DMA_CCR_TCIE;
	DMA1_Channel1->CCR |= DMA_Mode_Circular;

	//Tamaño del buffer a transmitir
	DMA1_Channel1->CNDTR = 5;

	//Address del periferico
	DMA1_Channel1->CPAR = (uint32_t) &ADC1->DR;

	//Address en memoria
	DMA1_Channel1->CMAR = (uint32_t) &adc_ch[0];

	//Enable
	//DMA1_Channel1->CCR |= DMA_CCR_EN;
}


