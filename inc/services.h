/*
 * services.h
 *
 *  Created on: Dec 28, 2014
 *      Author: daymoon
 */

#ifndef INCLUDES_SERVICES_H_
#define INCLUDES_SERVICES_H_

#include "stm32f0xx.h"
#include "delay.h"
#include "leds.h"
#include "scheduler.h"
#include "powerbit.h"
#include "commands.h"

#define MILISEKUND 		/1000

// periody jsou v milisekundach, neboli zakladni periode SysTick casovace
#define LED_SERVICE_PERIOD				(SCHEDULERPERIOD * 500 MILISEKUND)
#define REGULATORS_SERVICE_PERIOD		(SCHEDULERPERIOD * 10 MILISEKUND)
#define BUTTON_DEBOUNCE_PERIOD			(SCHEDULERPERIOD * 50 MILISEKUND)
#define AUTOMAT_PERIOD					(SCHEDULERPERIOD * 100 MILISEKUND)
#define AUTOMAT_TIMER_SHORT				(SCHEDULERPERIOD * 1000 MILISEKUND)
#define AUTOMAT_BEEPER_SHORT			(SCHEDULERPERIOD * 200 MILISEKUND)
#define AUTOMAT_BEEPER_LONG				(SCHEDULERPERIOD * 1000 MILISEKUND)

#define FREE			0
#define STARTED			127
#define REQUESTED		64


// sluzby
void LED_service(void);
void Delay_service(void);
void Regulators_service(void);
void Command_service(void);
void Button_UP_service(void);
void Automat_service(void);
void Automat_timer_service(void);
void Automat_beeper_service(void);


#endif /* INCLUDES_SERVICES_H_ */
