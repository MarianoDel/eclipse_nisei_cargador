/*
 * adc.h
 *
 *  Created on: 10/09/2014
 *      Author: Mariano
 */

#ifndef ADC_H_
#define ADC_H_

#define RCC_ADC_CLK 		(RCC->APB2ENR & 0x00000200)
#define RCC_ADC_CLK_ON 		RCC->APB2ENR |= 0x00000200
#define RCC_ADC_CLK_OFF 	RCC->APB2ENR &= ~0x00000200

#define ADC_IT_ADRDY                               ADC_IER_ADRDYIE
#define ADC_IT_EOSMP                               ADC_IER_EOSMPIE
#define ADC_IT_EOC                                 ADC_IER_EOCIE
#define ADC_IT_EOSEQ                               ADC_IER_EOSEQIE
#define ADC_IT_OVR                                 ADC_IER_OVRIE
#define ADC_IT_AWD                                 ADC_IER_AWDIE

#define ADC_CH0		0x00000001
#define ADC_CH1		0x00000002
#define ADC_CH2		0x00000004

#define ADC_Channel_0                              ADC_CHSELR_CHSEL0
#define ADC_Channel_1                              ADC_CHSELR_CHSEL1
#define ADC_Channel_2                              ADC_CHSELR_CHSEL2
#define ADC_Channel_3                              ADC_CHSELR_CHSEL3
#define ADC_Channel_4                              ADC_CHSELR_CHSEL4
#define ADC_Channel_5                              ADC_CHSELR_CHSEL5
#define ADC_Channel_6                              ADC_CHSELR_CHSEL6
#define ADC_Channel_7                              ADC_CHSELR_CHSEL7
#define ADC_Channel_8                              ADC_CHSELR_CHSEL8
#define ADC_Channel_9                              ADC_CHSELR_CHSEL9
#define ADC_Channel_10                             ADC_CHSELR_CHSEL10 /*!< Not available for STM32F031 devices */
#define ADC_Channel_11                             ADC_CHSELR_CHSEL11 /*!< Not available for STM32F031 devices */
#define ADC_Channel_12                             ADC_CHSELR_CHSEL12 /*!< Not available for STM32F031 devices */
#define ADC_Channel_13                             ADC_CHSELR_CHSEL13 /*!< Not available for STM32F031 devices */
#define ADC_Channel_14                             ADC_CHSELR_CHSEL14 /*!< Not available for STM32F031 devices */
#define ADC_Channel_15                             ADC_CHSELR_CHSEL15 /*!< Not available for STM32F031 devices */
#define ADC_Channel_16                             ADC_CHSELR_CHSEL16
#define ADC_Channel_17                             ADC_CHSELR_CHSEL17
#define ADC_Channel_18                             ADC_CHSELR_CHSEL18 /*!< Not available for STM32F030 devices */

#define ADC_Channel_TempSensor                     ((uint32_t)ADC_Channel_16)
#define ADC_Channel_Vrefint                        ((uint32_t)ADC_Channel_17)
#define ADC_Channel_Vbat                           ((uint32_t)ADC_Channel_18) /*!< Not available for STM32F030 devices */

#define ADC_SampleTime_1_5Cycles                     ((uint32_t)0x00000000)
#define ADC_SampleTime_7_5Cycles                     ((uint32_t)0x00000001)
#define ADC_SampleTime_13_5Cycles                    ((uint32_t)0x00000002)
#define ADC_SampleTime_28_5Cycles                    ((uint32_t)0x00000003)
#define ADC_SampleTime_41_5Cycles                    ((uint32_t)0x00000004)
#define ADC_SampleTime_55_5Cycles                    ((uint32_t)0x00000005)
#define ADC_SampleTime_71_5Cycles                    ((uint32_t)0x00000006)
#define ADC_SampleTime_239_5Cycles                   ((uint32_t)0x00000007)

#define ADC_Resolution_12b                         ((uint32_t)0x00000000)
#define ADC_Resolution_10b                         ADC_CFGR1_RES_0
#define ADC_Resolution_8b                          ADC_CFGR1_RES_1
#define ADC_Resolution_6b                          ADC_CFGR1_RES

#define ADC_ClockMode_AsynClk                  ((uint32_t)0x00000000)   /*!< ADC Asynchronous clock mode */
#define ADC_ClockMode_SynClkDiv2               ADC_CFGR2_CKMODE_0   /*!<  Synchronous clock mode divided by 2 */
#define ADC_ClockMode_SynClkDiv4               ADC_CFGR2_CKMODE_1   /*!<  Synchronous clock mode divided by 4 */


void AdcConfig (void);
unsigned short ReadADC1 (unsigned int);
unsigned short ReadADC1_SameSampleTime (unsigned int);
void SetADC1_SampleTime (void);
unsigned short ReadADC1Check (unsigned char);

#endif /* ADC_H_ */
