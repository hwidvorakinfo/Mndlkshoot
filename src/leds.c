/*
 * leds.c
 *
 *  Created on: Dec 28, 2014
 *      Author: daymoon
 */

#include "leds.h"

void leds_config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(LED_CLOCKS, ENABLE);

	// LED zelena
	GPIO_InitStructure.GPIO_Pin = GREEN_LED;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(LED_PORT, &GPIO_InitStructure);

	// LED cervena
	GPIO_InitStructure.GPIO_Pin = RED_LED;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(LED_PORT, &GPIO_InitStructure);

	GPIO_SetBits(LED_PORT, GREEN_LED);					// zelena LED off
	GPIO_SetBits(LED_PORT, RED_LED);					// cervena LED off


	// zalozeni ulohy blikani oranzove led
	if(Scheduler_Add_Task(LED_service, 0, LED_SERVICE_PERIOD) == SCH_MAX_TASKS)
	{
		// chyba pri zalozeni service
	}

}
