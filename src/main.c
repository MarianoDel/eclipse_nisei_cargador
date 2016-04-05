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
#include "stm32f0xx_conf.h"
#include "stm32f0xx_adc.h"
//#include "stm32f0xx_can.h"
//#include "stm32f0xx_cec.h"
//#include "stm32f0xx_comp.h"
//#include "stm32f0xx_crc.h"
//#include "stm32f0xx_crs.h"
//#include "stm32f0xx_dac.h"
//#include "stm32f0xx_dbgmcu.h"
//#include "stm32f0xx_dma.h"
//#include "stm32f0xx_exti.h"
//#include "stm32f0xx_flash.h"
#include "stm32f0xx_gpio.h"
//#include "stm32f0xx_i2c.h"
//#include "stm32f0xx_iwdg.h"
#include "stm32f0xx_misc.h"
//#include "stm32f0xx_pwr.h"
#include "stm32f0xx_rcc.h"
//#include "stm32f0xx_rtc.h"
#include "stm32f0xx_spi.h"
//#include "stm32f0xx_syscfg.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_usart.h"
//#include "stm32f0xx_wwdg.h"
#include "system_stm32f0xx.h"
#include "stm32f0xx_it.h"

//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>


//--- My includes ---//
#include "stm32f0x_gpio.h"
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

// ------- Externals de los Timers -------
volatile unsigned short wait_ms_var = 0;
volatile unsigned short prog_timer = 0;


// ------- Externals de los modos -------




//--- VARIABLES GLOBALES ---//


// ------- de los timers -------
volatile unsigned short timer_standby;
volatile unsigned short timer_dmx_display_show;
volatile unsigned char display_timer;
volatile unsigned char filter_timer;
static __IO uint32_t TimingDelay;

//volatile unsigned char door_filter;
//volatile unsigned char take_sample;
//volatile unsigned char move_relay;
//volatile unsigned char secs = 0;
//volatile unsigned short minutes = 0;

// ------- del display -------
unsigned char v_opt [10];


// ------- del DMX -------
volatile unsigned char signal_state = 0;
volatile unsigned char dmx_timeout_timer = 0;
//unsigned short tim_counter_65ms = 0;

// ------- de los filtros DMX -------
#define LARGO_F		32
#define DIVISOR_F	5
unsigned char vd0 [LARGO_F + 1];
unsigned char vd1 [LARGO_F + 1];
unsigned char vd2 [LARGO_F + 1];
unsigned char vd3 [LARGO_F + 1];
unsigned char vd4 [LARGO_F + 1];


#define IDLE	0
#define LOOK_FOR_BREAK	1
#define LOOK_FOR_MARK	2
#define LOOK_FOR_START	3

#define RCC_DMA_CLK (RCC->AHBENR & RCC_AHBENR_DMAEN)
#define RCC_DMA_CLK_ON 		RCC->AHBENR |= RCC_AHBENR_DMAEN
#define RCC_DMA_CLK_OFF 	RCC->AHBENR &= ~RCC_AHBENR_DMAEN


//--- FUNCIONES DEL MODULO ---//
void TimingDelay_Decrement(void);
void DMAConfig(void);

void Update_PWM (unsigned short);


//unsigned char GetValue (unsigned char * , const unsigned short * );
unsigned short GetValue (unsigned char * );



#define DMX_TIMEOUT	20
unsigned char MAFilter (unsigned char, unsigned char *);

//--- FILTROS DE SENSORES ---//
#define LARGO_FILTRO 16
#define DIVISOR      4   //2 elevado al divisor = largo filtro
//#define LARGO_FILTRO 32
//#define DIVISOR      5   //2 elevado al divisor = largo filtro
unsigned short vtemp [LARGO_FILTRO + 1];
unsigned short vpote [LARGO_FILTRO + 1];

//--- FIN DEFINICIONES DE FILTRO ---//


//-------------------------------------------//
// @brief  Main program.
// @param  None
// @retval None
//------------------------------------------//
int main(void)
{
	unsigned char i;
	unsigned short ii;
	unsigned char onsync = 0;

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

			//LED1_ON;
			//Update_TIM1_CH2 (V_GRID_SENSE >> 2);
			//LED1_OFF;
			if (LEDR)
				LEDR_OFF;
			else
				LEDR_ON;

		}
	}

	//--- Fin Prueba ADC y DMA ---//














	return 0;
}
//--- End of Main ---//


/*
unsigned char GetValue (unsigned char * pn, const unsigned short * new_val)
{
	unsigned char i;
	unsigned char colon = 0;

	//me fijo la posiciones de la , o ;
	for (i = 0; i < 6; i++)
	{
		if ((*(pn + i) == ',') || ((*(pn + i) == ';')))
		{
			colon = i;
			i = 6;
		}
	}

	if ((colon == 0) || (colon >= 5))
		return 0;

	switch (colon)
	{
		case 1:
			*new_val = *pn - '0';
			break;

		case 2:
			*new_val = (*pn - '0') * 10 + (*(pn + 1) - '0');
			break;

		case 3:
			*new_val = (*pn - '0') * 100 + (*(pn + 1) - '0') * 10 + (*(pn + 2) - '0');
			break;

		case 4:
			*new_val = (*pn - '0') * 1000 + (*(pn + 1) - '0') * 100 + (*(pn + 2) - '0') * 10 + (*(pn + 2) - '0');
			break;

	}
	return 1;
}
*/

unsigned short GetValue (unsigned char * pn)
{
	unsigned char i;
	unsigned char colon = 0;
	unsigned short new_val = 0xffff;

	//me fijo la posiciones de la , o ;
	for (i = 0; i < 6; i++)
	{
		if ((*(pn + i) == ',') || ((*(pn + i) == ';')))
		{
			colon = i;
			i = 6;
		}
	}

	if ((colon == 0) || (colon >= 5))
		return 0;

	switch (colon)
	{
		case 1:
			new_val = *pn - '0';
			break;

		case 2:
			new_val = (*pn - '0') * 10 + (*(pn + 1) - '0');
			break;

		case 3:
			new_val = (*pn - '0') * 100 + (*(pn + 1) - '0') * 10 + (*(pn + 2) - '0');
			break;

		case 4:
			new_val = (*pn - '0') * 1000 + (*(pn + 1) - '0') * 100 + (*(pn + 2) - '0') * 10 + (*(pn + 2) - '0');
			break;

	}
	return new_val;
}

void Update_PWM (unsigned short pwm)
{
	Update_TIM3_CH1 (pwm);
	Update_TIM3_CH2 (4095 - pwm);
}

void UpdateDMX (unsigned char * pckt, unsigned short ch, unsigned char val)
{
	if ((ch > 0) && (ch < 512))
		*(pckt + ch) = val;
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

	if (display_timer)
		display_timer--;

	if (timer_standby)
		timer_standby--;

	if (dmx_timeout_timer)
		dmx_timeout_timer--;

	if (timer_dmx_display_show)
		timer_dmx_display_show--;

	if (prog_timer)
		prog_timer--;

//	if (take_sample)
//		take_sample--;

	if (filter_timer)
		filter_timer--;


	/*
	//cuenta 1 segundo
	if (button_timer_internal)
		button_timer_internal--;
	else
	{
		if (button_timer)
		{
			button_timer--;
			button_timer_internal = 1000;
		}
	}
	*/
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


