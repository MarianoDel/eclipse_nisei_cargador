/*
 * hard.h
 *
 *  Created on: 28/11/2013
 *      Author: Mariano
 */

#ifndef HARD_H_
#define HARD_H_


//-------- Defines For Configuration -------------

//#define VOLTAGE_ZERO		2048	//---- Valores de tension del ADC
#define VOLTAGE_ZERO		1950	//zero corregido 1.76V
//#define VOLTAGE_MIN			2250
#define VOLTAGE_MIN			(VOLTAGE_ZERO + 150)
#define VOLTAGE_SYNC_ON		100
#define VOLTAGE_SYNC_OFF	50

//#define PEAK_CURRENT_SET	600		//(0.8V en pata 1 opamp) 25A pico
//#define PEAK_CURRENT_SET	1240		//(1V en pata 1 opamp) 31A pico
#define PEAK_CURRENT_SET	2250		//(1V en pata 1 opamp) 35A pico
//#define PEAK_CURRENT_SET	2000		//2A pico
#define VBAT_MIN_SET		VOLTAGE_MIN

//-------- End Of Defines For Configuration ------
//GPIOA pin0	ADC FASE
//GPIOA pin1	ADC VBAT
//GPIOA pin2	ADC I_OUT
//GPIOA pin3	ADC TEMP
//GPIOA pin4	ADC SET_LOAD

//GPIOA pin5
//GPIOA pin6
//GPIOA pin7

//GPIOA pin8
#define MOSFET ((GPIOA->ODR & 0x0100) == 0)
#define MOSFET_OFF	GPIOA->BSRR = 0x00000100
#define MOSFET_ON GPIOA->BSRR = 0x01000000

//GPIOA pin9

//GPIOA pin10	usart tx rx
#define LEDG ((GPIOA->ODR & 0x0400) != 0)
#define LEDG_ON	GPIOA->BSRR = 0x00000400
#define LEDG_OFF GPIOA->BSRR = 0x04000000

//GPIOA pin11
#define LEDY ((GPIOA->ODR & 0x0800) != 0)
#define LEDY_ON	GPIOA->BSRR = 0x00000800
#define LEDY_OFF GPIOA->BSRR = 0x08000000

//GPIOA pin12
#define LEDR ((GPIOA->ODR & 0x1000) != 0)
#define LEDR_ON	GPIOA->BSRR = 0x00001000
#define LEDR_OFF GPIOA->BSRR = 0x10000000

//GPIOA pin13
//GPIOA pin14
//GPIOA pin15

//GPIOB pin0
//GPIOB pin1
//GPIOB pin3
//GPIOB pin4
//GPIOB pin5
#define SYNC ((GPIOB->ODR & 0x0020) != 0)
#define SYNC_ON	GPIOB->BSRR = 0x00000020
#define SYNC_OFF GPIOB->BSRR = 0x00200000

//GPIOB pin6
//GPIOB pin7



//ESTADOS DEL PROGRAMA PRINCIPAL
//estados del MAIN MENU
enum var_main_states
{
	MAIN_STANDBY = 0,
	MAIN_CHARGING,
	MAIN_ERROR_IPEAK,
	MAIN_ERROR_VIN,
	MAIN_ERROR_VBAT,
	MAIN_ERROR_VBAT_REVERSAL,
	MAIN_ERROR_TEMP

};

//estados de ERRORES
enum var_error_states
{
	ERROR_NO = 0,
	ERROR_IPEAK,
	ERROR_VIN,
	ERROR_VBAT,
	ERROR_VBAT_REVERSAL,
	ERROR_TEMP,
	ERROR_RUN,
	ERROR_RUN_A,
	ERROR_RUN_B

};

//respuestas de las funciones
#define RESP_NO			0
#define RESP_YES		1

//---- Temperaturas en el LM335
//37	2,572
//40	2,600
//45	2,650
//50	2,681
//55	2,725
//60	2,765
#define TEMP_IN_30		3226
#define TEMP_IN_35		3279
#define TEMP_IN_50		3434
#define TEMP_IN_65		3591

//#define VTEMP_NTC_HIGH		60			//1K 90°
//#define VTEMP_NTC_NORMAL	117			//2K 65°
#define VTEMP_NTC_HIGH		200			//2.5K 	pruebas
#define VTEMP_NTC_NORMAL	265			//5k 	pruebas
#define VTEMP_NTC_LOW		525			//10K 20°	600mV

//---- Timeout de los errores
#define TT_ERROR_VBAT	2000
#define TT_ERROR_VBAT_REVERSAL	3000
#define TT_ERROR_TEMP	3000
#define TT_BIP_LONG		1000
#define TT_BIP_SHORT	300

#define TT_CHARGING		60		//60 segs para volver a standby



// ------- de los switches -------
void UpdateErrors (void);
void ErrorCommands(unsigned char);


#endif /* HARD_H_ */
