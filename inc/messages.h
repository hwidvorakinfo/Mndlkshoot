/*
 * messages.h
 *
 *  Created on: 5. 11. 2019
 *      Author: Petr Dvorak
 */

#ifndef MESSAGES_H_
#define MESSAGES_H_

#include "max7219.h"

extern uint8_t message0_reset[XSIZE*YSIZE];
extern uint8_t message1_ready[XSIZE*YSIZE];
extern uint8_t message2_3[XSIZE*YSIZE];
extern uint8_t message3_2[XSIZE*YSIZE];
extern uint8_t message4_1[XSIZE*YSIZE];
extern uint8_t message5_go[XSIZE*YSIZE];
extern uint8_t message6_1won[XSIZE*YSIZE];
extern uint8_t message7_2won[XSIZE*YSIZE];
extern uint8_t message9_winner1[XSIZE*YSIZE];
extern uint8_t message10_winner2[XSIZE*YSIZE];

#endif /* MESSAGES_H_ */
