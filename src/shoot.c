/*
 * shoot.c
 *
 *  Created on: 31. 10. 2019
 *      Author: Petr Dvorak
 */

#include "shoot.h"
#include "services.h"
#include "defs.h"
#include "messages.h"

const uint8_t font6x6_Table[] = {
  // nula '0'
         0x78, 0xCC, 0xCC, 0xCC, 0xCC, 0x78,
  // jedna '1'
         0x60, 0xE0, 0x60, 0x60, 0x60, 0xF0,
  // dve '2'
         0x78, 0xCC, 0x0C, 0x78, 0xC0, 0xFC,
  // tri '3'
         0x78, 0xCC, 0x18, 0x0C, 0xCC, 0x78,
  // ctyri '4'
         0x38, 0x78, 0xD8, 0xFC, 0x18, 0x18,
  // pet '5'
         0xFC, 0xC0, 0xF8, 0x0C, 0xCC, 0xF8,
  // sest '6'
         0x78, 0xC0, 0xF8, 0xCC, 0xCC, 0x78,
  // sedm '7'
         0xF8, 0x18, 0x18, 0x30, 0x60, 0x60,
  // osm '8'
         0x78, 0xCC, 0x78, 0xCC, 0xCC, 0x78,
  // devet '9'
         0x78, 0xCC, 0xCC, 0x7C, 0x0C, 0x78,
  // dvojtecka ':' 10
         0x00, 0x00, 0xC0, 0x00, 0xC0, 0x00,
  // prazdny znak ' ' 11
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // mikrotecka '.' 12
         0x80, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define FONT6X6SIZE			6
const uint8_t font6x6_Sizes[] = {
  // '0' '1' '2' '3' '4' '5' '6' '7' '8' '9' ':' ' ' '.'
      6,  4,  6,  6,  6,  6,  6,  5,  6,  6,  6,  6,  1
};

const uint8_t font4x5_Table[] = {
  // nula '0'
         0x60, 0x90, 0x90, 0x90, 0x60,
  // jedna '1'
         0x40, 0xC0, 0x40, 0x40, 0xE0,
  // dve '2'
         0x60, 0x90, 0x20, 0x40, 0xF0,
  // tri '3'
         0x60, 0x90, 0x20, 0x90, 0x60,
  // ctyri '4'
         0x20, 0x60, 0xA0, 0xF0, 0x20,
  // pet '5'
         0xF0, 0x80, 0xE0, 0x10, 0xE0,
  // sest '6'
         0x60, 0x80, 0xE0, 0x90, 0x60,
  // sedm '7'
         0xE0, 0x10, 0x10, 0x20, 0x20,
  // osm '8'
         0x60, 0x90, 0x60, 0x90, 0x60,
  // devet '9'
         0x60, 0x90, 0x70, 0x10, 0x20,
  // dvojtecka ':' 10
         0x00, 0x80, 0x00, 0x80, 0x00,
  // prazdny znak ' ' 11
         0x00, 0x00, 0x00, 0x00, 0x00,
  // mikrotecka '.' 12
         0x00, 0x00, 0x00, 0x00, 0x01,
};
#define FONT4X5SIZE			5

uint8_t score_frame[XSIZE*YSIZE] = {	0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00};	// frame buffer pro zobrazeni na displeji 4 sloupce po 8 bitech a 8 radcich

static STATES automat = STATE_RESET;
static uint8_t score[2] = {0, 0};
static uint8_t round = 0;
static uint8_t button = FALSE;
static uint8_t button_request = FALSE;
static uint8_t waiting_timer = FALSE;
static uint8_t round_finished = FALSE;
static uint8_t round_winner = 0;

void shoot_set_timer(uint16_t time);
void shoot_display_round_winner(uint8_t round_winner);
void shoot_total_winner(void);
void shoot_display_message(MESSAGES msg);
void shoot_beeper_init(void);
void shoot_set_beeper(uint16_t time);

void inputs_outputs_config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHBPeriphClockCmd(INPUTS_CLOCKS, ENABLE);

	// vstupy
	GPIO_InitStructure.GPIO_Pin = TARGET1 | TARGET2 | BUTTON_UP | BUTTON_DOWN | BUTTON_OK;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(INPUTS_PORT, &GPIO_InitStructure);

	// vystup casovace pro beeper
	GPIO_InitStructure.GPIO_Pin = BUZZER;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(BUZZER_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(BUZZER_PORT, BUZZER_SOURCE, BUZZER_AF);

	shoot_beeper_init();
	shoot_beeper_off();

	// nastaveni preruseni na hrany vstupu
	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	// tlacitko
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);		// Connect EXTI0 Line to PA0 pin
	EXTI_InitStructure.EXTI_Line = BUTTON_UP_EXTI;						// Configure EXTI line
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_1_IRQn;					// Enable and set EXTI0 Interrupt
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// cil 1
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource2);		// Connect EXTI2 Line to PA2 pin
	EXTI_InitStructure.EXTI_Line = TARGET1_EXTI;						// Configure EXTI line
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_3_IRQn;					// Enable and set EXTI2 Interrupt
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// cil 2
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4);		// Connect EXTI4 Line to PA4 pin
	EXTI_InitStructure.EXTI_Line = TARGET2_EXTI;						// Configure EXTI line
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;					// Enable and set EXTI4 Interrupt
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void shoot_config(void)
{
	// zalozeni sluzby pro stavovy automat
	if(Scheduler_Add_Task(Automat_service, 0, AUTOMAT_PERIOD) == SCH_MAX_TASKS)
	{
		// chyba pri zalozeni service
	}
}

void shoot_set_timer(uint16_t time)
{
	// zalozeni sluzby pro casovou smycku stavoveho automatu
	if(Scheduler_Add_Task(Automat_timer_service, time, 0) == SCH_MAX_TASKS)
	{
		// chyba pri zalozeni service
	}

	shoot_set_timer_state(FALSE);			// nulovani stavu casovace
}

void shoot_set_beeper(uint16_t time)
{
	// zalozeni sluzby pro beeper
	if(Scheduler_Add_Task(Automat_beeper_service, time, 0) == SCH_MAX_TASKS)
	{
		// chyba pri zalozeni service
	}

	shoot_beeper_on();						// zapni beeper
}

void shoot_set_timer_state(uint8_t state)
{
	waiting_timer = state;
}

void shoot_automat(void)
{
	// obsluha stavoveho automatu
	switch (automat)
	{
		case STATE_RESET:
			// inicializace hry
			score[0] = 0;
			score[1] = 0;
			round = 1;

			display_clear();						// vymazani displeje
			shoot_display_message(MESSAGE0_RESET);	// zobrazeni zpravy
			button = FALSE;							// nulovani stavu tlacitka
			button_request = FALSE;
			automat = STATE_TOREADY;				// dalsi stav automatu
		break;

		case STATE_TOREADY:
			if (TRUE == button)
			{
				automat = STATE_READY;				// pokud bylo stisknuto tlacitko, nastav dalsi stav
			}
		break;

		case STATE_READY:
			display_clear();						// vymazani displeje
			shoot_display_message(MESSAGE1_READY);	// zobrazeni zpravy
			button = FALSE;							// nulovani stavu tlacitka
			button_request = FALSE;
			automat = STATE_TOSTART;				// dalsi stav automatu
		break;

		case STATE_TOSTART:
			if (TRUE == button)
			{
				automat = STATE_3;					// pouze cekani na odstartovani kola, pokud bylo stisknuto tlacitko, nastav dalsi stav = zacatek odpocitavani
			}
		break;

		case STATE_3:
			display_clear();						// vymazani displeje
			shoot_display_message(MESSAGE2_3);			// zobrazeni zpravy
			button = FALSE;							// nulovani stavu tlacitka
			button_request = FALSE;
			shoot_set_timer(AUTOMAT_TIMER_SHORT);	// zaloz cekaci smycku
			shoot_set_beeper(AUTOMAT_BEEPER_SHORT);	// kratky beep
			automat = STATE_3TO2;					// dalsi stav automatu
		break;

		case STATE_3TO2:
			if (TRUE == waiting_timer)
			{
				automat = STATE_2;					// dalsi stav automatu
			}
		break;

		case STATE_2:
			display_clear();						// vymazani displeje
			shoot_display_message(MESSAGE3_2);			// zobrazeni zpravy
			button = FALSE;							// nulovani stavu tlacitka
			button_request = FALSE;
			shoot_set_timer(AUTOMAT_TIMER_SHORT);	// zaloz cekaci smycku
			shoot_set_beeper(AUTOMAT_BEEPER_SHORT);	// kratky beep
			automat = STATE_2TO1;					// dalsi stav automatu
		break;

		case STATE_2TO1:
			if (TRUE == waiting_timer)
			{
				automat = STATE_1;					// dalsi stav automatu
			}
		break;

		case STATE_1:
			display_clear();						// vymazani displeje
			shoot_display_message(MESSAGE4_1);		// zobrazeni zpravy
			button = FALSE;							// nulovani stavu tlacitka
			button_request = FALSE;
			shoot_set_timer(AUTOMAT_TIMER_SHORT);	// zaloz cekaci smycku
			shoot_set_beeper(AUTOMAT_BEEPER_SHORT);	// kratky beep
			automat = STATE_1TOGO;					// dalsi stav automatu
		break;

		case STATE_1TOGO:
			if (TRUE == waiting_timer)
			{
				automat = STATE_GO;					// dalsi stav automatu
			}
		break;

		case STATE_GO:
			display_clear();						// vymazani displeje
			shoot_display_message(MESSAGE5_GO);		// zobrazeni zpravy
			button = FALSE;							// nulovani stavu tlacitka
			button_request = FALSE;
			round_finished = FALSE;					// ridici promennou hry vynuluj
			round_winner = 0;						// vynuluj viteze kola pro jeho urceni
			shoot_set_beeper(AUTOMAT_BEEPER_LONG);	// dlouhy beep
			automat = STATE_GAME;					// dalsi stav automatu
		break;

		case STATE_GAME:
			if (TRUE == round_finished)				// na zaklade ridici promenne nastav dalsi prubeh
			{
				shoot_display_round_winner(round_winner);	// nastav displej podle viteze kola
				shoot_set_beeper(AUTOMAT_BEEPER_LONG);	// dlouhy beep
				automat = STATE_ROUNDWINNER;		// dalsi stav automatu
			}
		break;

		case STATE_ROUNDWINNER:
			if (TRUE == button)
			{
				automat = STATE_SCORE;				// dalsi stav je zobrazeni skore
			}
		break;

		case STATE_SCORE:
			display_clear();						// vymazani displeje
			shoot_display_message(MESSAGE8_SCORE);	// zobrazeni zpravy
			button = FALSE;							// nulovani stavu tlacitka
			button_request = FALSE;
			automat = STATE_TONEXTROUND;			// dalsi stav automatu
		break;

		case STATE_TONEXTROUND:
			if (TRUE == button)
			{
				round++;							// zvys pocitadlo kol
				automat = STATE_NEXTROUND;			// dalsi stav automatu
			}
		break;

		case STATE_NEXTROUND:
			if (round <= MAXROUNDS)
			{
				// muzeme jit do dalsiho kola
				automat = STATE_READY;
			}
			else
			{
				// jiz je konec hry
				automat = STATE_WINNER;
			}
		break;

		case STATE_WINNER:
			display_clear();						// vymazani displeje
			shoot_total_winner();					// urci viteze
			button = FALSE;							// nulovani stavu tlacitka
			button_request = FALSE;
			automat = STATE_TORESET;				// dalsi stav automatu
		break;

		case STATE_TORESET:
			if (TRUE == button)
			{
				automat = STATE_RESET;				// dalsi stav automatu
			}
		break;

		default:
			automat = STATE_RESET;					// nouzovy stav
		break;
	}
}

void shoot_display_round_winner(uint8_t round_winner)
{
	display_clear();						// vymazani displeje
	button = FALSE;							// nulovani stavu tlacitka

	if (PLAYER1 == round_winner)
	{
		shoot_display_message(MESSAGE6_1WON);		// zobrazeni zpravy o vitezi PLAYER1
	}
	else if (PLAYER2 == round_winner)
	{
		shoot_display_message(MESSAGE7_2WON);		// zobrazeni zpravy o vitezi PLAYER2
	}
}

void shoot_total_winner(void)
{
	display_clear();						// vymazani displeje
	button = FALSE;							// nulovani stavu tlacitka

	if (score[0] > score[1])
	{
		shoot_display_message(MESSAGE9_WINNER1);		// zobrazeni zpravy o celkovem vitezi PLAYER1
	}
	else if (score[0] < score[1])
	{
		shoot_display_message(MESSAGE10_WINNER2);		// zobrazeni zpravy o celkovem vitezi PLAYER2
	}
}

void shoot_button_event(void)
{
	button = TRUE;
}

void shoot_round_winner(uint8_t roundwinner)
{
	// volano z ISR

	round_finished = TRUE;							// kolo skoncilo
	round_winner = roundwinner;						// oznac viteze kola

	if (roundwinner == PLAYER1)
	{
		score[0]++;									// vitez kola je PLAYER1, zvys mu skore
	}
	else if (roundwinner == PLAYER2)
	{
		score[1]++;									// vitez kola je PLAYER2, zvys mu skore
	}
}

uint8_t shoot_get_round_state(void)
{
	return round_finished;
}

uint8_t shoot_get_button_request(void)
{
	return button_request;
}

void shoot_set_button_request(void)
{
	button_request = TRUE;
}

void shoot_display_message(MESSAGES msg)
{
	uint8_t i;

	switch (msg)
	{
		case MESSAGE0_RESET:
			display_fill_display(&message0_reset[0]);	// vychozi displej hry
		break;

		case MESSAGE1_READY:
			display_fill_display(&message1_ready[0]);	// priprav se
		break;

		case MESSAGE2_3:
			display_fill_display(&message2_3[0]);		// 3...
		break;

		case MESSAGE3_2:
			display_fill_display(&message3_2[0]);		// 2...
		break;

		case MESSAGE4_1:
			display_fill_display(&message4_1[0]);		// 1...
		break;

		case MESSAGE5_GO:
			display_fill_display(&message5_go[0]);		// GO !
		break;

		case MESSAGE6_1WON:
			display_fill_display(&message6_1won[0]);	// WON: 1
		break;

		case MESSAGE7_2WON:
			display_fill_display(&message7_2won[0]);	// WON: 2
		break;

		case MESSAGE8_SCORE:
			// vypis score podle aktualniho stavu
			for (i = 0; i < FONT4X5SIZE; i++)
			{
				// vypis leve cifry jako je ve fontu
				if (score[0] == 1)
				{
					// pokud je jednicka vlevo, posun o 3 pixely doprava, kvuli zarovnani
					score_frame[5+i*XSIZE] = font4x5_Table[FONT4X5SIZE*score[0]+i] >> 3;
				}
				else
				{	// kazdy jiny znak zobraz normalne posuno
					score_frame[5+i*XSIZE] = font4x5_Table[FONT4X5SIZE*score[0]+i] >> 2;
				}

				// vypis prave cifry posunute o 2 bity doprava
				score_frame[6+i*XSIZE] = font4x5_Table[FONT4X5SIZE*score[1]+i] >> 2;
			}

			// vypis dvojtecky do LSB leve cifry a do MSB prave cifry
			score_frame[9] = score_frame[9] | 0x01;
			score_frame[17] = score_frame[17] | 0x01;

			score_frame[10] = score_frame[10] | 0x80;
			score_frame[18] = score_frame[18] | 0x80;

			// vypis pozadavku na tlacitko
			score_frame[3] = score_frame[3] | 0x01;
			score_frame[7] = score_frame[7] | 0x01;

			display_fill_display(&score_frame[0]);	// N : M
		break;

		case MESSAGE9_WINNER1:
			display_fill_display(&message9_winner1[0]);	// > 1 <
		break;

		case MESSAGE10_WINNER2:
			display_fill_display(&message10_winner2[0]);// > 2 <
		break;

		default:
			display_fill_display(&message0_reset[0]);	// vychozi displej hry
		break;
	}
}

void shoot_beeper_init(void)
{
	volatile uint32_t TimerPeriod = 0;
	volatile uint16_t Channel1Pulse = 0;
	volatile TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	volatile TIM_OCInitTypeDef  TIM_OCInitStructure;

	  /* TIM1 Configuration ---------------------------------------------------
	   Generate 7 PWM signals with 4 different duty cycles:
	   TIM1 input clock (TIM1CLK) is set to APB2 clock (PCLK2)
	    => TIM1CLK = PCLK2 = SystemCoreClock
	   TIM1CLK = SystemCoreClock, Prescaler = 0, TIM1 counter clock = SystemCoreClock
	   SystemCoreClock is set to 48 MHz for STM32F0xx devices

	   The objective is to generate 7 PWM signal at 17.57 KHz:
	     - TIM1_Period = (SystemCoreClock / 17570) - 1
	   The channel 1 and channel 1N duty cycle is set to 50%
	   The channel 2 and channel 2N duty cycle is set to 37.5%
	   The channel 3 and channel 3N duty cycle is set to 25%
	   The channel 4 duty cycle is set to 12.5%
	   The Timer pulse is calculated as follows:
	     - ChannelxPulse = DutyCycle * (TIM1_Period - 1) / 100

	   Note:
	    SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f0xx.c file.
	    Each time the core clock (HCLK) changes, user had to call SystemCoreClockUpdate()
	    function to update SystemCoreClock variable value. Otherwise, any configuration
	    based on this variable will be incorrect.
	  ----------------------------------------------------------------------- */

	/* Compute the value to be set in ARR regiter to generate signal frequency at pwmfreq Hz */
	TimerPeriod = (SYSTEMCORECLOCK / BEEPERFREQ ) - 1;

	/* Compute CCR3 value to generate a duty cycle at PWM_DUTY_INIT%  for channel 3 and 3N */
	Channel1Pulse = (uint16_t) (((uint32_t) BEEPER_DUTY * (TimerPeriod - 1)) / BEEPER_DUTY_MAX);

	/* BEEPER_TIM clock enable */
	RCC_APB1PeriphClockCmd(BEEPER_TIM_CLOCKS , ENABLE);

	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = BEEPER_TIM_PRESC;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(BEEPER_TIM, (TIM_TimeBaseInitTypeDef *)&TIM_TimeBaseStructure);

	/* Channel 1 Configuration in PWM mode */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	TIM_OCInitStructure.TIM_Pulse = Channel1Pulse;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
	TIM_OC1Init(BEEPER_TIM, (TIM_OCInitTypeDef *)&TIM_OCInitStructure);

	TIM_OC1PreloadConfig(BEEPER_TIM, TIM_OCPreload_Enable);

	/* BEEPER_TIM counter enable */
	TIM_Cmd(BEEPER_TIM, ENABLE);
	delay_ms(200);
	TIM_Cmd(BEEPER_TIM, DISABLE);
	delay_ms(200);
	TIM_Cmd(BEEPER_TIM, ENABLE);
	delay_ms(200);
	TIM_Cmd(BEEPER_TIM, DISABLE);
}

void shoot_beeper_on(void)
{
	TIM_Cmd(BEEPER_TIM, ENABLE);
}

void shoot_beeper_off(void)
{
	TIM_Cmd(BEEPER_TIM, DISABLE);
}
