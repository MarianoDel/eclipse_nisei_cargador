/**
  ******************************************************************************
  * @file    Template_2/stm32f0_gpio.c
  * @author  Nahuel
  * @version V1.0
  * @date    22-August-2014
  * @brief   GPIO functions.
  ******************************************************************************
  * @attention
  *
  * Use this functions to configure global inputs/outputs.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "stm32f0xx.h"
#include "hard.h"



//--- Private typedef ---//
//--- Private define ---//
//--- Private macro ---//
//--- Private variables ---//
//--- Private function prototypes ---//
//--- Private functions ---//

//-------------------------------------------//
// @brief  GPIO configure.
// @param  None
// @retval None
//------------------------------------------//
void GPIO_Config (void)
{
	unsigned long temp;


	//--- MODER ---//
	//00: Input mode (reset state)
	//01: General purpose output mode
	//10: Alternate function mode
	//11: Analog mode

	//--- OTYPER ---//
	//These bits are written by software to configure the I/O output type.
	//0: Output push-pull (reset state)
	//1: Output open-drain

	//--- ORSPEEDR ---//
	//These bits are written by software to configure the I/O output speed.
	//x0: Low speed.
	//01: Medium speed.
	//11: High speed.
	//Note: Refer to the device datasheet for the frequency.

	//--- PUPDR ---//
	//These bits are written by software to configure the I/O pull-up or pull-down
	//00: No pull-up, pull-down
	//01: Pull-up
	//10: Pull-down
	//11: Reserved


#ifdef GPIOA_ENABLE

	//--- GPIO A ---//
	if (!GPIOA_CLK)
		GPIOA_CLK_ON;


	temp = GPIOA->MODER;	//2 bits por pin
	temp &= 0xFC0CFC00;		//PA0 - PA4 analog input; PA8 PA10 PA11 PA12 output push pull
	temp |= 0x015103FF;		//
	GPIOA->MODER = temp;

	temp = GPIOA->OTYPER;	//1 bit por pin
	temp &= 0xFFFFFEFF;
	temp |= 0x00000100;		//PA8 open drain
	GPIOA->OTYPER = temp;

	temp = GPIOA->OSPEEDR;	//2 bits por pin
	temp &= 0xFC0CFFFF;
	temp |= 0x00000000;		//PA8 PA10 PA11 PA12 low speed
	GPIOA->OSPEEDR = temp;

	temp = GPIOA->PUPDR;	//2 bits por pin
	temp &= 0xFFFFFFFF;
	temp |= 0x00000000;		//
	GPIOA->PUPDR = temp;

	//Alternate Fuction
	//en algunos perifericos necesito primero mandar el clk y luego la funciona alternativa
	//como USART2
	//GPIOA->AFR[0] = 0x00001100;	//PA3 -> AF1; PA2 -> AF1
	//GPIOA->AFR[1] = 0x00000110;	//PA10 -> AF1; PA9 -> AF1

#endif

#ifdef GPIOB_ENABLE

	//--- GPIO B ---//
	if (!GPIOB_CLK)
		GPIOB_CLK_ON;

	temp = GPIOB->MODER;	//2 bits por pin
	temp &= 0xFFFFF3FF;		//PB5 output
	temp |= 0x00000400;
	GPIOB->MODER = temp;

	temp = GPIOB->OTYPER;	//1 bit por pin
	temp &= 0xFFFFFFFF;
	temp |= 0x00000000;
	GPIOB->OTYPER = temp;

	temp = GPIOB->OSPEEDR;	//2 bits por pin
	temp &= 0xFFFFF3FF;
	temp |= 0x00000000;		//PB5 low speed
	GPIOB->OSPEEDR = temp;

	temp = GPIOB->PUPDR;	//2 bits por pin
	temp &= 0xFFFFFFFF;
	temp |= 0x00000000;
	GPIOB->PUPDR = temp;

	//Alternate Fuction
	//GPIOB->AFR[0] = 0x11000000;	//PA7 -> AF1; PA6 -> AF1

#endif

#ifdef GPIOF_ENABLE

	//--- GPIO F ---//
	if (!GPIOF_CLK)
		GPIOF_CLK_ON;

	temp = GPIOF->MODER;
	temp &= 0xFFFFFFFF;
	temp |= 0x00000000;
	GPIOF->MODER = temp;

	temp = GPIOF->OTYPER;
	temp &= 0xFFFFFFFF;
	temp |= 0x00000000;
	GPIOF->OTYPER = temp;

	temp = GPIOF->OSPEEDR;
	temp &= 0xFFFFFFFF;
	temp |= 0x00000000;
	GPIOF->OSPEEDR = temp;

	temp = GPIOF->PUPDR;
	temp &= 0xFFFFFFFF;
	temp |= 0x00000000;
	GPIOF->PUPDR = temp;

#endif


	//EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	//EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	//EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	//EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	//EXTI_Init(&EXTI_InitStructure);
//	EXTI_InitStructure.EXTI_Line = EXTI_Line8;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure);

		//GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);

	// Enable and set Button EXTI Interrupt to the lowest priority
	//NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	//NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

//	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

//	NVIC_Init(&NVIC_InitStructure);

}

inline void EXTIOff (void)
{
	EXTI->IMR &= ~0x00000100;
}

inline void EXTIOn (void)
{
	EXTI->IMR |= 0x00000100;
}

//--- end of file ---//
