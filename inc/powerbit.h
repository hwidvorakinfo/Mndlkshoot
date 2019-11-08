/*
 * powerbit.h
 *
 *  Created on: 28. 4. 2019
 *      Author: daymoon
 */

#ifndef POWERBIT_H_
#define POWERBIT_H_

#include "stm32f0xx.h"
#include "scheduler.h"
#include "defs.h"
#include "services.h"
#include "powerbit_types.h"
#include "stdio.h"

#define OUTPUT_VOLTAGE			520

#define LOW_PORT				GPIOB
#define LOW_PORT_CLOCKS			RCC_AHBPeriph_GPIOB
#define LOW_PORT_PIN			GPIO_Pin_1
#define LOW_PIN_SOURCE			GPIO_PinSource1

#define HIGH_PORT				GPIOA
#define HIGH_PORT_CLOCKS		RCC_AHBPeriph_GPIOA
#define HIGH_PORT_PIN			GPIO_Pin_10
#define HIGH_PIN_SOURCE			GPIO_PinSource10

// vstupy ADC
#define ADC_INPUT_CLOCKS		RCC_AHBPeriph_GPIOA
#define ADC_CLOCKS				RCC_APB2Periph_ADC1
#define DMA_CLOCKS				RCC_AHBPeriph_DMA1
#define IN_PORT					GPIOA
#define IN_CURRENT				GPIO_Pin_0
#define IN_VOLTAGE				GPIO_Pin_1
#define IN_ADC_INPUTS			IN_CURRENT | IN_VOLTAGE

#define IN_ADC					ADC1
#define IN_CURRENT_CHAN			ADC_Channel_0
#define IN_VOLTAGE_CHAN			ADC_Channel_1
#define IN_ADC_COUNT			2
#define IN_ADC_DMA_CHAN			DMA1_Channel1
#define IN_ADC_DMA_IRQHandler	DMA1_CH1_IRQHandler

#define SYSTEMCORECLOCK			48000000
#define PWMFREQ					150000
#define PWMFREQMAX				1000000

#define PWM_TIM					TIM1
#define PWM_TIM_CLOCKS			RCC_APB2Periph_TIM1
#define PWM_TIM_PRESCALER		0
// deadtime = TFCLK * PWM_TIM_DEADTIME
#define PWM_TIM_DEADTIME		10

#define INTREGMAX				1000
#define INTREGMIN				(-INTREGMAX)
#define PSDVALUEUPPERLIMIT		5
#define PSDVALUELOWERLIMIT		(-PSDVALUEUPPERLIMIT)
#define PSDMAX					5000
#define PSDMIN					(-PSDMAX)
#define PSD_KP					50
#define PSD_KI					25
#define PSD_KD					10

#define VOLT_MULT				44600
#define VOLT_DIV				100000
#define VOLT_OFFSET				0
#define VOLTAGEMAX				5000
#define CURR_MULT				403
#define CURR_DIV				1000
#define CURR_OFFSET				0
#define CURRENTMAX				4095

#define VOLTAGE					1
#define CURRENT					0

#define VOLTAVRGCOUNT			128

#define CURRENTPEAKLIMIT		2500
#define CURRENTDATALEN			2
#define CURRENTMAXPOS			0
#define CURRENTDATAPOS			1

#define SWDAC_PHASE_MAX			2

//
// Buck - rozsah duty od 0 do 95%
// Boost - rozsah duty od 0 do 95%
// Flyback - rozsah duty od 0 do 95%
// Forward - rozsah duty od 0 do 50%
// Cuk - uzky rozsah vstupniho napeti, rozsah duty od 0 do 95%
// Push-Pull /- rozsah duty od 5 do 50%
//
#define PWM_DUTY_INIT			2500
#define PWM_DUTY_MAX			9500
#define PWM_DUTY_MIN			0
#define PWM_MAX					10000
#define PWM_DUTY_STEP			100


void powerbit_init(void);
void powerbit_gpio_init(void);
void powerbit_adcdma_init(void);
void powerbit_wait_for_adc(void);
void powerbit_curr_peak(void);
void powerbit_volt_avrg(void);
void powerbit_regulator_process(void);
void powerbit_psd_calculation(void);
void powerbit_pwm_init(void);
void powerbit_enable(void);
void powerbit_disable(void);
void powerbit_set_duty(uint16_t duty);
void powerbit_set_voltage(uint16_t voltage);
void powerbit_set_current(uint16_t current);
void powerbit_set_freq(uint32_t freq);
void powerbit_set_dutymax(uint16_t duty);
void powerbit_set_dutymin(uint16_t duty);
void powerbit_set_const(uint8_t type, int16_t value);


uint16_t powerbit_get_voltage(void);
uint16_t powerbit_get_peak(void);
powerbit_output_t *powerbit_get_output(void);


#endif /* POWERBIT_H_ */
