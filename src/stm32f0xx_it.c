/**
  ******************************************************************************
  * @file    Project/STM32F0xx_StdPeriph_Templates/stm32f0xx_it.c 
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    05-December-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_it.h"
#include "shoot.h"

/** @addtogroup Template_Project
  * @{
  */

extern usart_data_t Tx;
extern usart_data_t Rx;


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
	while(1)
	{
	}
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
	while(1)
	{
	}
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
	while(1)
	{
	}
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	// spust pruchod schedulerem
	Run_scheduler();
}

/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f0xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles DMA1 Channel 1 interrupt request.
  * @param  None
  * @retval None
  */
void IN_ADC_DMA_IRQHandler(void)
{
	// mereni a prenos do pameti je hotovy
	if(DMA_GetITStatus(DMA1_IT_TC1))
	{
		// zkontroluj maximalni proud
		powerbit_curr_peak();

		// pridej vzorek napeti do prumeru
		powerbit_volt_avrg();

		// vymaz priznak preruseni
		DMA_ClearITPendingBit(DMA1_IT_TC1);
	}
}

/**
* @brief  This function handles USART interrupt request.
* @param  None
* @retval None
*/
void USART1_IRQHandler(void)
{
  /* USART in mode Transmitter -------------------------------------------------*/
  if (USART_GetITStatus(USART, USART_IT_TXE) == SET)
  {
        USART_SendData(USART, Tx.buffer[Tx.index++]);
        if (Tx.buffer[Tx.index] == 0)
        {
        	// konec prenosu
        	Tx.status = TRANSFERDONE;

        	/* Disable the USARTx transmit data register empty interrupt */
        	USART_ITConfig(USART, USART_IT_TXE, DISABLE);
        }
  }

  /* USART in mode Receiver --------------------------------------------------*/
  if (USART_GetITStatus(USART, USART_IT_RXNE) == SET)
  {
	  if (Rx.status != RECEIVEDONE)
	  {
		  Rx.buffer[Rx.index] = USART_ReceiveData(USART);
		  if ((Rx.buffer[Rx.index] == '\r') || (Rx.buffer[Rx.index] == '\n'))
		  {
			  // konec prenosu
			  Rx.status = RECEIVEDONE;
			  Rx.buffer[Rx.index] = 0;

			  // zalozeni sluzby pro dekodovani a zpracovani zpravy
			  if(Scheduler_Add_Task(Command_service, 0, 0) == SCH_MAX_TASKS)
			  {
				// chyba pri zalozeni service
			  }
		  }
		  else
		  {
			  // prijmi dalsi znak
			  Rx.index++;
		  }
	  }
  }
}

void EXTI0_1_IRQHandler(void)
{
	if(EXTI_GetITStatus(BUTTON_UP_EXTI) != RESET)
	{
		//if (shoot_get_button_request() == FALSE)
		{
			// zpracovani udalosti stisku tlacitka UP
			// zalozeni sluzby pro zpracovani stisku tlacitka UP
			if(Scheduler_Add_Task(Button_UP_service, BUTTON_DEBOUNCE_PERIOD, 0) == SCH_MAX_TASKS)
			{
				// chyba pri zalozeni service
			}

			shoot_set_button_request();


		}

		/* Clear the EXTI line pending bit */
		EXTI_ClearITPendingBit(BUTTON_UP_EXTI);
	}
}

void EXTI2_3_IRQHandler(void)
{
	if(EXTI_GetITStatus(TARGET1_EXTI) != RESET)
	{
		if (shoot_get_round_state() == FALSE)
		{
			// zpracovani udalosti cile 1
			shoot_round_winner(PLAYER1);
		}
		/* Clear the EXTI line pending bit */
		EXTI_ClearITPendingBit(TARGET1_EXTI);
	}
}

void EXTI4_15_IRQHandler(void)
{
	if(EXTI_GetITStatus(TARGET2_EXTI) != RESET)
	{
		if (shoot_get_round_state() == FALSE)
		{
			// zpracovani udalosti cile 2
			shoot_round_winner(PLAYER2);
		}
		/* Clear the EXTI line pending bit */
		EXTI_ClearITPendingBit(TARGET2_EXTI);
	}
}


/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
