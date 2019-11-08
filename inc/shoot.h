/*
 * shoot.h
 *
 *  Created on: 31. 10. 2019
 *      Author: Petr Dvorak
 */

#ifndef SHOOT_H_
#define SHOOT_H_

#include "stm32f0xx.h"
#include "scheduler.h"
#include "defs.h"

#define INPUTS_CLOCKS		RCC_AHBPeriph_GPIOA
#define INPUTS_PORT			GPIOA
#define TARGET1				GPIO_Pin_2
#define TARGET2				GPIO_Pin_4
#define BUTTON_UP			GPIO_Pin_0
#define BUTTON_DOWN			GPIO_Pin_1
#define BUTTON_OK			GPIO_Pin_10
#define BUTTON_UP_EXTI		EXTI_Line0
#define TARGET1_EXTI		EXTI_Line2
#define TARGET2_EXTI		EXTI_Line4

#define BUZZER_CLOCK		RCC_AHBPeriph_GPIOA
#define BUZZER_PORT			GPIOA
#define BUZZER				GPIO_Pin_6
#define BUZZER_SOURCE		GPIO_PinSource6
#define BUZZER_AF			GPIO_AF_1

#define BEEPERFREQ			2400
#define BEEPER_DUTY			5000
#define BEEPER_DUTY_MAX		10000
#define BEEPER_TIM			TIM3
#define BEEPER_TIM_CLOCKS	RCC_APB1Periph_TIM3
#define BEEPER_TIM_PRESC	0


#define PLAYER1				10
#define PLAYER2				20

#define MAXROUNDS			5

// stavovy automat
typedef enum {
	STATE_RESET = 0,
	STATE_TOREADY,
	STATE_READY,
	STATE_TOSTART,
	STATE_3,
	STATE_3TO2,
	STATE_2,
	STATE_2TO1,
	STATE_1,
	STATE_1TOGO,
	STATE_GO,
	STATE_GAME,
	STATE_ROUNDWINNER,
	STATE_SCORE,
	STATE_TONEXTROUND,
	STATE_NEXTROUND,
	STATE_WINNER,
	STATE_TORESET
} STATES;

// zpravy na displeji
typedef enum {
	MESSAGE0_RESET = 0,
	MESSAGE1_READY,
	MESSAGE2_3,
	MESSAGE3_2,
	MESSAGE4_1,
	MESSAGE5_GO,
	MESSAGE6_1WON,
	MESSAGE7_2WON,
	MESSAGE8_SCORE,
	MESSAGE9_WINNER1,
	MESSAGE10_WINNER2
} MESSAGES;

extern const uint8_t font6x6_Table[];
extern const uint8_t font6x6_Sizes[];

void shoot_automat(void);
void inputs_outputs_config(void);
void shoot_config(void);
void shoot_button_event(void);
void shoot_set_timer_state(uint8_t state);
void shoot_round_winner(uint8_t roundwinner);
uint8_t shoot_get_round_state(void);
uint8_t shoot_get_button_request(void);
void shoot_set_button_request(void);
void shoot_beeper_on(void);
void shoot_beeper_off(void);


#endif /* SHOOT_H_ */
