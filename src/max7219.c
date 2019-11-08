/*
 * max7219.c
 *
 *  Created on: 31. 10. 2019
 *      Author: Petr Dvorak
 */

#include "max7219.h"
#include "delay.h"

uint16_t buffer[4];									// pracovni buffer pro poslani prikazu
uint8_t frame[XSIZE*YSIZE] = {	0x00, 0x00, 0x00, 0x00,
								0x00, 0x00, 0x00, 0x00,
								0x00, 0x00, 0x00, 0x00,
								0x00, 0x00, 0x00, 0x00,
								0x00, 0x00, 0x00, 0x00,
								0x00, 0x00, 0x00, 0x00,
								0x00, 0x00, 0x00, 0x00,
								0x00, 0x00, 0x00, 0x00};	// frame buffer pro zobrazeni na displeji 4 sloupce po 8 bitech a 8 radcich


void CS_active(void);
void CS_inactive(void);
void max7219_send_cmd(uint16_t cmd);
void max7219_send_4_cmds(uint16_t *cmd);

void display_config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

	delay_ms(1000);

	RCC_AHBPeriphClockCmd(CS_CLOCKS | SPI_PORT_CLOCKS, ENABLE);

	// signaly SPI
	GPIO_InitStructure.GPIO_Pin = SCK_PIN | MOSI_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(SPI_PORT, &GPIO_InitStructure);

	GPIO_PinAFConfig(SPI_PORT, SCK_SOURCE, SCK_AF);
	GPIO_PinAFConfig(SPI_PORT, MOSI_SOURCE, MOSI_AF);

	// vystup CS
	GPIO_InitStructure.GPIO_Pin = CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(CS_PORT, &GPIO_InitStructure);

	CS_inactive();										// CS do L

	// inicializace SPI rozhrani
	RCC_APB2PeriphClockCmd(DISPLAY_SPI_CLOCKS, ENABLE);

	SPI_I2S_DeInit(DISPLAY_SPI);
	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_Init(DISPLAY_SPI, &SPI_InitStructure);
	SPI_Cmd(DISPLAY_SPI, ENABLE);

	// zadne dekodovani pro digity 0-7
	buffer[0] = DECODE + 0x00;
	buffer[1] = DECODE + 0x00;
	buffer[2] = DECODE + 0x00;
	buffer[3] = DECODE + 0x00;
	max7219_send_4_cmds(&buffer[0]);

	// scan digity 0-7
	buffer[0] = SCANLIMIT + 0x07;
	buffer[1] = SCANLIMIT + 0x07;
	buffer[2] = SCANLIMIT + 0x07;
	buffer[3] = SCANLIMIT + 0x07;
	max7219_send_4_cmds(&buffer[0]);

	// normal rezim
	buffer[0] = TEST + 0x00;
	buffer[1] = TEST + 0x00;
	buffer[2] = TEST + 0x00;
	buffer[3] = TEST + 0x00;
	max7219_send_4_cmds(&buffer[0]);

	// normal operation
	buffer[0] = SHUTDOWN + 0x01;
	buffer[1] = SHUTDOWN + 0x01;
	buffer[2] = SHUTDOWN + 0x01;
	buffer[3] = SHUTDOWN + 0x01;
	max7219_send_4_cmds(&buffer[0]);

	// nastaveni intenzity 0-15
	display_set_intensity(0);

	// vymazani displeje
	display_clear();

	display_send_frame();
}

void CS_inactive(void)
{
	GPIO_SetBits(CS_PORT, CS_PIN);					// CS = L
}

void CS_active(void)
{
	GPIO_ResetBits(CS_PORT, CS_PIN);				// CS = H
}

void max7219_send_cmd(uint16_t cmd)
{
    /* Waiting until TX FIFO is empty */
    while (SPI_GetTransmissionFIFOStatus(DISPLAY_SPI) != SPI_TransmissionFIFOStatus_Empty)
    {}

    /* Wait busy flag */
    while(SPI_I2S_GetFlagStatus(DISPLAY_SPI, SPI_I2S_FLAG_BSY) == SET)
    {}

    SPI_I2S_SendData16(DISPLAY_SPI, cmd);
}

void max7219_send_4_cmds(uint16_t *cmd)
{
	max7219_send_cmd(cmd[0]);
	max7219_send_cmd(cmd[1]);
	max7219_send_cmd(cmd[2]);
	max7219_send_cmd(cmd[3]);

    /* Waiting until TX FIFO is empty */
    while (SPI_GetTransmissionFIFOStatus(DISPLAY_SPI) != SPI_TransmissionFIFOStatus_Empty)
    {}

    /* Wait busy flag */
    while(SPI_I2S_GetFlagStatus(DISPLAY_SPI, SPI_I2S_FLAG_BSY) == SET)
    {}

	CS_active();
	CS_inactive();
}

void display_set_intensity(uint8_t intensity)
{
	// intenzita je cislo od 0 do 15
	intensity %= 16;

	buffer[0] = INTENSITY + intensity;
	buffer[1] = INTENSITY + intensity;
	buffer[2] = INTENSITY + intensity;
	buffer[3] = INTENSITY + intensity;
	max7219_send_4_cmds(&buffer[0]);
}

void display_send_frame(void)
{
	// poslani celeho frame bufferu do displeje
	uint8_t x, y;

	for (y = 0; y < YSIZE; y++)
	{
		for (x = 0; x < XSIZE; x++)
		{
			buffer[x] = (DIGIT0 + (y << 8)) + frame[XSIZE*y + x];
			max7219_send_4_cmds(&buffer[0]);
		}
	}
}

void display_fill_display(uint8_t *image)
{
	// napleni celeho frame bufferu novym obsahem
	uint8_t x, y;

	for (y = 0; y < YSIZE; y++)
	{
		for (x = 0; x < XSIZE; x++)
		{
			buffer[x] = (DIGIT0 + (y << 8)) + image[XSIZE*y + x];
			max7219_send_4_cmds(&buffer[0]);
		}
	}
}

void display_clear(void)
{
	// poslani celeho prazdneho frame bufferu do displeje
	uint8_t x, y;

	for (y = 0; y < YSIZE; y++)
	{
		for (x = 0; x < XSIZE; x++)
		{
			buffer[x] = (DIGIT0 + (y << 8)) + 0;
			max7219_send_4_cmds(&buffer[0]);
		}
	}
}
