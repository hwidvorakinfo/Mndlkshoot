/*
 * usart.c
 *
 *  Created on: 3. 5. 2019
 *      Author: daymoon
 */

#include "usart.h"
#include "string.h"

volatile usart_data_t Tx;
volatile usart_data_t Rx;

void usart_config(void)
{
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  const uint8_t hello[] = "-= MdlkShoot v.1 =-";

  /* Enable GPIO clock */
  RCC_AHBPeriphClockCmd(USART_TX_CLOCKS | USART_RX_CLOCKS, ENABLE);

  /* Enable USART clock */
  RCC_APB2PeriphClockCmd(USART_CLOCKS, ENABLE);

  /* Connect PB6 to USART1_Tx */
  GPIO_PinAFConfig(USART_TX_PORT, USART_TX_SOURCE, USART_TX_AF);

  /* Connect PB7 to USART1_Rx */
  GPIO_PinAFConfig(USART_RX_PORT, USART_RX_SOURCE, USART_RX_AF);

  /* Configure USART Tx and Rx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = USART_TX_PIN;
  GPIO_Init(USART_TX_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = USART_RX_PIN;
  GPIO_Init(USART_RX_PORT, &GPIO_InitStructure);

  /* USARTx configuration ----------------------------------------------------*/
  /* USARTx configured as follow:
  - BaudRate = 115200 baud
  - Word Length = 8 Bits
  - One Stop Bit
  - No parity
  - Hardware flow control disabled (RTS and CTS signals)
  - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART, &USART_InitStructure);

  /* NVIC configuration */
  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable USART */
  USART_Cmd(USART, ENABLE);

  /* Enable the USARTx Receive interrupt: this interrupt is generated when the USARTx receive data register is not empty */
  USART_ITConfig(USART, USART_IT_RXNE, ENABLE);

  // inicializuj data prijmu a vysilani
  Tx.index = 0;
  Tx.status = READYTOSEND;
  Rx.index = 0;
  Rx.status = READYTORECEIVE;

  // uvodni vypis
  usart_send_text((uint8_t *)&hello);
  usart_newline();
}

void usart_send_text(uint8_t *text)
{
	while (Tx.status == TRANSFERING)
	{;}

	Tx.index = 0;
	Tx.status = TRANSFERING;

	strcpy((char *)&Tx.buffer, (const char *)text);

	/* Enable the USARTx transmit data register empty interrupt */
	USART_ITConfig(USART, USART_IT_TXE, ENABLE);
}

void usart_newline(void)
{
	while (Tx.status == TRANSFERING)
	{;}

	uint8_t endline[] = "  ";

	Tx.index = 0;
	Tx.status = TRANSFERING;

	endline[0] = '\r';
	endline[1] = '\n';
	strcpy((char *)&Tx.buffer, (const char *)endline);

	/* Enable the USARTx transmit data register empty interrupt */
	USART_ITConfig(USART, USART_IT_TXE, ENABLE);
}

void usart_release_Rx_buffer()
{
	Rx.index = 0;
	Rx.status = READYTORECEIVE;
}

uint8_t *get_Rx_buffer(void)
{
	return (uint8_t *)&Rx.buffer;
}
