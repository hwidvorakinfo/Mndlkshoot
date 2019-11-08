/*
 * max7219.h
 *
 *  Created on: 31. 10. 2019
 *      Author: Petr Dvorak
 */

#ifndef MAX7219_H_
#define MAX7219_H_

#include "stm32f0xx.h"
#include "scheduler.h"
#include "defs.h"

#define CS_CLOCKS			RCC_AHBPeriph_GPIOB
#define CS_PORT				GPIOB
#define CS_PIN				GPIO_Pin_1

#define SPI_PORT_CLOCKS		RCC_AHBPeriph_GPIOA
#define SPI_PORT			GPIOA
#define SCK_PIN				GPIO_Pin_5
#define MOSI_PIN			GPIO_Pin_7
#define SCK_SOURCE			GPIO_PinSource5
#define MOSI_SOURCE			GPIO_PinSource7
#define SCK_AF		        GPIO_AF_0
#define MOSI_AF		        GPIO_AF_0

#define DISPLAY_SPI			SPI1
#define DISPLAY_SPI_CLOCKS	RCC_APB2Periph_SPI1

// prikazy MAX7219
#define NOOP				0x0000
#define DIGIT0				0x0100
#define DIGIT1				0x0200
#define DIGIT2				0x0300
#define DIGIT3				0x0400
#define DIGIT4				0x0500
#define DIGIT5				0x0600
#define DIGIT6				0x0700
#define DIGIT7				0x0800
#define DECODE				0x0900
#define INTENSITY			0x0A00
#define SCANLIMIT			0x0B00
#define SHUTDOWN			0x0C00
#define TEST				0x0F00

#define YSIZE				8
#define XSIZE				4

void display_config(void);
void display_set_intensity(uint8_t intensity);
void display_send_frame(void);
void display_clear(void);
void display_fill_display(uint8_t *image);

#endif /* MAX7219_H_ */
