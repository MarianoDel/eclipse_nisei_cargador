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
unsigned short vd0 [LARGO_F];
unsigned short vd1 [LARGO_F];
unsigned short vd2 [LARGO_F];
unsigned short vd3 [LARGO_F];
unsigned short vd4 [LARGO_F];
#define VIN_VECTOR			vd0
#define VBAT_VECTOR			vd1
#define IPEAK_VECTOR		vd2
#define TEMP_VECTOR			vd3
#define VSETLOAD_VECTOR		vd4

unsigned char filter_index = 0;


#define RCC_DMA_CLK (RCC->AHBENR & RCC_AHBENR_DMAEN)
#define RCC_DMA_CLK_ON 		RCC->AHBENR |= RCC_AHBENR_DMAEN
#define RCC_DMA_CLK_OFF 	RCC->AHBENR &= ~RCC_AHBENR_DMAEN



//--- FUNCIONES DEL MODULO ---//
void DMAConfig (void);
unsigned char CheckPolarityReversal (void);
unsigned short GetVBAT (void);
unsigned short GetTEMP (void);
unsigned short GetVSETLOAD (void);
void UpdateFilters (void);


//-------------------------------------------//
// @brief  Main program.
// @param  None
// @retval None
//------------------------------------------//
int main(void)
{
	unsigned char i;
	unsigned char onsync = 0, enab = 0;
	unsigned char mosfet_sync = 0;
	enum var_main_states main_state = MAIN_STANDBY;
	unsigned short vbat_local = 0;
	unsigned short vsetload_local = 0;

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

	//PRUEBA MOSFET
	/*
	while (1)
	{
		if (MOSFET)
		{
			MOSFET_OFF;
			LEDY_OFF;
		}
		else
		{
			MOSFET_ON;
			LEDY_ON;
		}

		Wait_ms(1000);
	}
	*/
	//FIN PRUEBA MOSFET

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
	//TIM_14_Init();
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

	//--- COMIENZO Synchro ADC y DMA ---//
	if ((ADC1->CR & ADC_CR_ADSTART) != 0) 		//Ensure that no conversion on going
	{
		ADC1->CR |= ADC_CR_ADSTP; 				//Stop any ongoing conversion
	}

	while ((ADC1->CR & ADC_CR_ADSTP) != 0);		//Wait until ADSTP is reset by hardware i.e. conversion is stopped
	ADC1->CR |= ADC_CR_ADDIS;					//Disable the ADC
	while ((ADC1->CR & ADC_CR_ADEN) != 0);		//Wait until the ADC is fully disabled

	DMA1_Channel1->CCR |= DMA_CCR_EN;			//habilito el DMA
	TIM1->CR1 |= TIM_CR1_CEN;					//habilito el TIM1

	ADC1->CR |= ADC_CR_ADEN;					//Enable the ADC
	while ((ADC1->ISR & ADC_ISR_ADRDY) == 0);	//Wait until ADC ready

	ADC1->CR |= ADC_CR_ADSTART;			//para que pueda disparar por TIM1
	//--- FIN Synchro ADC y DMA ---//

	//--- Prueba Synchro ADC y DMA ---//
	/*
	while (1)
	{
		if (DMA1->ISR & DMA1_FLAG_TC1)
		{
		    // Clear DMA TC flag
			DMA1->IFCR = DMA1_FLAG_TC1;
			if ((LEDR) && (adc_ch[3] > 3500))	//es el pullup de TEMP
			{
				LEDR_OFF;
				MOSFET_OFF;
			}
			else
			{
				LEDR_ON;
				MOSFET_ON;
			}
		}
	}
	*/
	//--- Fin Prueba Synchro ADC y DMA ---//

	//--- Comienzo Programa de Produccion ---//

	while(1)
	{
		//me fijo si hubo overrun
//		if (ADC1->ISR & ADC_IT_OVR)
//		{
//			ADC1->ISR |= ADC_IT_EOC | ADC_IT_EOSEQ | ADC_IT_OVR;
//			if (LEDY)
//				LEDY_OFF;
//			else
//				LEDY_ON;
//		}

		if (DMA1->ISR & DMA1_FLAG_TC1)
		{
			LEDG_ON;
		    // Clear DMA TC flag
			DMA1->IFCR = DMA1_FLAG_TC1;

			switch (main_state)
			{
				case MAIN_STANDBY:
					//si tengo bateria conectada y necesita carga, lo hago
					MOSFET_OFF;
					vbat_local = GetVBAT();
					vsetload_local = GetVSETLOAD();
					if ((vbat_local > VBAT_MIN_SET) && (vbat_local < vsetload_local))
						main_state = MAIN_CHARGING;

					if (CheckPolarityReversal() == RESP_YES)
					{
						main_state = MAIN_ERROR_VBAT_REVERSAL;
						ErrorCommands(ERROR_VBAT_REVERSAL);
						timer_standby = TT_ERROR_VBAT_REVERSAL;
					}
					break;

				case MAIN_CHARGING:
					//reviso no pasarme de corriente si estoy con el mosfet activo
					if ((IPEAK > PEAK_CURRENT_SET) && (MOSFET))
					{
						MOSFET_OFF;
						LEDY_OFF;
						LEDR_ON;
						timer_standby = 2;
						main_state = MAIN_ERROR_IPEAK;
						break;
					}

					vbat_local = GetVBAT();
					vsetload_local = GetVSETLOAD();

					if (vbat_local > vsetload_local)		//cuento 60 segs y vuelvo a standby
					{
						MOSFET_OFF;
						LEDY_OFF;
						secs++;

						if (secs > TT_CHARGING)
							main_state = MAIN_STANDBY;
					}
					else
					{
						//prendo mosfet siempre onsync
						if (mosfet_sync)
						{
							MOSFET_ON;
							LEDY_ON;
							LEDR_OFF;	//para salir de IPEAK
						}

						if (secs)
							secs--;
					}

//					if (vbat_local < VBAT_MIN_SET)
//					{
//						MOSFET_OFF;
//						LEDY_OFF;
//						ErrorCommands(ERROR_VBAT);
//						main_state = MAIN_ERROR_VBAT;
//						timer_standby = TT_ERROR_VBAT;
//					}

					break;

				case MAIN_ERROR_IPEAK:
					//a diferencia de otros casos reestablezco al toque el error
					if (!timer_standby)
					{
						main_state = MAIN_STANDBY;
						LEDR_OFF;
					}

					break;

				case MAIN_ERROR_VIN:
					break;

				case MAIN_ERROR_VBAT:
					if (!timer_standby)
					{
						if (GetVBAT() > VOLTAGE_MIN)
						{
							ErrorCommands(ERROR_NO);
							main_state = MAIN_STANDBY;
						}
						else
							timer_standby = TT_ERROR_VBAT;
					}
					break;

				case MAIN_ERROR_VBAT_REVERSAL:
					if (!timer_standby)
					{
						if (CheckPolarityReversal() == RESP_NO)
						{
							ErrorCommands(ERROR_NO);
							main_state = MAIN_STANDBY;
						}
						else
							timer_standby = TT_ERROR_VBAT_REVERSAL;
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
//			if (main_state < MAIN_ERROR_IPEAK)
//			{
//			}

			//Verifico fase con ADC
			if (VIN > VOLTAGE_SYNC_ON)
			{
				if (!SYNC)	//si venia de no estar en SYNC pongo el flag
					mosfet_sync = 1;
				else
					mosfet_sync = 0;

				SYNC_ON;
			}
			else if (VIN < VOLTAGE_SYNC_OFF)
				SYNC_OFF;

			LEDG_OFF;							//---- hasta aca 13.2us
		}

		//Resuelvo cuestiones que no tengan que ver con las muestras
		UpdateFilters();

		//muestro el led de error segun error_state
		UpdateErrors();

		//LEDG_OFF;								//---- hasta aca 16us
	}

	//--- Fin Prueba ADC y DMA ---//
	return 0;
}
//--- End of Main ---//


unsigned char CheckPolarityReversal (void)
{
	//reviso el ultimo valor de VBAT
	if (VBAT < VOLTAGE_ZERO)	//debe ser tension negativa en la bateria
		return RESP_YES;
	else
		return RESP_NO;
}

unsigned short GetVBAT (void)
{

#if (LARGO_F == 32)
	return MAFilter32New(VBAT_VECTOR);
#endif

#if (LARGO_F == 16)
	return MAFilter16New(VBAT);
#endif

#if (LARGO_F == 8)
	return MAFilter8New(VBAT);
#endif

}

unsigned short GetTEMP (void)
{

#if (LARGO_F == 32)
	return MAFilter32New(TEMP_VECTOR);
#endif

#if (LARGO_F == 16)
	return MAFilter16New(VBAT);
#endif

#if (LARGO_F == 8)
	return MAFilter8New(VBAT);
#endif

}

unsigned short GetVSETLOAD (void)
{

#if (LARGO_F == 32)
	return MAFilter32New(VSETLOAD_VECTOR);
#endif

#if (LARGO_F == 16)
	return MAFilter16New(VBAT);
#endif

#if (LARGO_F == 8)
	return MAFilter8New(VBAT);
#endif

}

void UpdateFilters (void)
{
	if (filter_index < LARGO_F)
	{
		vd0 [filter_index] = adc_ch[0];
		vd1 [filter_index] = adc_ch[1];
		vd2 [filter_index] = adc_ch[2];
		vd3 [filter_index] = adc_ch[3];
		vd4 [filter_index] = adc_ch[4];

		filter_index++;
	}
	else
	{
		vd0 [0] = adc_ch[0];
		vd1 [0] = adc_ch[1];
		vd2 [0] = adc_ch[2];
		vd3 [0] = adc_ch[3];
		vd4 [0] = adc_ch[4];

		filter_index = 0;
	}
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



