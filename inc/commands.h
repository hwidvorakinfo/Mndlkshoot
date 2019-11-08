/*
 * commands.h
 *
 *  Created on: 7. 5. 2019
 *      Author: daymoon
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "stm32f0xx.h"
#include "defs.h"
#include "usart.h"
#include "powerbit.h"

typedef enum {
	COMMANDOK = 0,
	COMMANDWRONG
} COMMAND_STATUS;

typedef struct {
	uint8_t *command;
	COMMAND_STATUS (*p_itemfunc)(void *);
	uint8_t enabled;
} command_t;

void commands_process(void);
void commands_wrong_cmd(void);
void commands_ok_cmd(void);

COMMAND_STATUS cmd_powerbit_on(void *p_i);
COMMAND_STATUS cmd_powerbit_off(void *p_i);
COMMAND_STATUS cmd_powerbit_volt(void *p_i);
COMMAND_STATUS cmd_powerbit_curr(void *p_i);
COMMAND_STATUS cmd_powerbit_stat(void *p_i);
COMMAND_STATUS cmd_powerbit_freq(void *p_i);
COMMAND_STATUS cmd_powerbit_dutymax(void *p_i);
COMMAND_STATUS cmd_powerbit_dutymin(void *p_i);
COMMAND_STATUS cmd_powerbit_const(void *p_i);

#endif /* COMMANDS_H_ */
