/*
 * leds.h
 *
 *  Created on: Dec 28, 2014
 *      Author: daymoon
 */

#ifndef INCLUDES_LEDS_H_
#define INCLUDES_LEDS_H_

#include "stm32f0xx.h"
#include "scheduler.h"
#include "defs.h"
#include "services.h"

#define LED_PORT	GPIOF
#define LED_CLOCKS	RCC_AHBPeriph_GPIOF
#define GREEN_LED	GPIO_Pin_1
#define RED_LED		GPIO_Pin_0

extern void leds_config(void);

#endif /* INCLUDES_LEDS_H_ */
