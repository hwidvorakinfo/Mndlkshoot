/*
 * powerbit.c
 *
 *  Created on: 28. 4. 2019
 *      Author: daymoon
 */

#include "powerbit.h"

static volatile uint16_t dma_data[IN_ADC_COUNT];
static volatile uint16_t volt_average[VOLTAVRGCOUNT];
static volatile powerbit_regulator_t regulator = {OUTPUT_VOLTAGE, PSD_KP, PSD_KI, PSD_KD, 0, 0, 0, CURRENTPEAKLIMIT, TRUE};
static volatile powerbit_output_t output = {OUTPUT0, LOW_PORT, LOW_PORT_PIN, PHASE0, PWM_DUTY_INIT, PWM_DUTY_MIN, PWM_DUTY_MAX, INIT};
static volatile uint32_t pwmfreq = PWMFREQ;
static volatile TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
static volatile TIM_OCInitTypeDef  TIM_OCInitStructure;
static volatile TIM_BDTRInitTypeDef TIM_DeadTimeStructure;

void powerbit_init(void)
{
	powerbit_gpio_init();
	powerbit_adcdma_init();
	powerbit_pwm_init();

	// zalozeni ulohy vypoctu regulatoru
	if(Scheduler_Add_Task(Regulators_service, 0, REGULATORS_SERVICE_PERIOD) == SCH_MAX_TASKS)
	{
		// chyba pri zalozeni service
	}
}

void powerbit_gpio_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(LOW_PORT_CLOCKS + HIGH_PORT_CLOCKS, ENABLE);

	// AF PORTY VYSTUPU CASOVACE
	// LOW
	GPIO_InitStructure.GPIO_Pin = LOW_PORT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(LOW_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(LOW_PORT, LOW_PORT_PIN);				// LOW off

	// HIGH
	GPIO_InitStructure.GPIO_Pin = HIGH_PORT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(HIGH_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(HIGH_PORT, HIGH_PORT_PIN);			// HIGH off

	GPIO_PinAFConfig(LOW_PORT, LOW_PIN_SOURCE, GPIO_AF_2);		// PB1 jako TIM1_CH3N
	GPIO_PinAFConfig(HIGH_PORT, HIGH_PIN_SOURCE, GPIO_AF_2);	// PA10 jako TIM1_CH3
}

void powerbit_adcdma_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// ADC + DMA
	// ADC vstup kanalu IN_PORT
	RCC_AHBPeriphClockCmd(ADC_INPUT_CLOCKS, ENABLE);
	RCC_APB2PeriphClockCmd(ADC_CLOCKS, ENABLE);
	GPIO_InitStructure.GPIO_Pin = IN_ADC_INPUTS;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(IN_PORT, &GPIO_InitStructure);

	// inicializace ADC1
	ADC_DeInit(IN_ADC);
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;
	ADC_Init(IN_ADC, &ADC_InitStructure);

	/* Convert the ADC1 Channel 3 with 239.5 Cycles as sampling time */
	ADC_ChannelConfig(IN_ADC, IN_CURRENT_CHAN, ADC_SampleTime_239_5Cycles);
	ADC_ChannelConfig(IN_ADC, IN_VOLTAGE_CHAN, ADC_SampleTime_239_5Cycles);
	ADC_GetCalibrationFactor(IN_ADC);

	ADC_DMARequestModeConfig(IN_ADC, ADC_DMAMode_Circular);
	ADC_DMACmd(IN_ADC, ENABLE);
	ADC_Cmd(IN_ADC, ENABLE);

	/* Wait the ADRDY flag */
	while(!ADC_GetFlagStatus(IN_ADC, ADC_FLAG_ADRDY));

	/* ADC1 regular Software Start Conv */
	ADC_StartOfConversion(IN_ADC);

	/* DMA1 clock enable */
	RCC_AHBPeriphClockCmd(DMA_CLOCKS, ENABLE);

	/* DMA1 Channel1 Config */
	DMA_DeInit(IN_ADC_DMA_CHAN);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_BASE+0x40;	// adresa ADC_DR registru, ADC1_BASE + ADC_DR address offset
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dma_data;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = IN_ADC_COUNT;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(IN_ADC_DMA_CHAN, &DMA_InitStructure);

	/* Enable DMA1 Channel2 Transfer Complete interrupt */
	DMA_ITConfig(IN_ADC_DMA_CHAN, DMA_IT_TC, ENABLE);

	/* Enable DMA1 channel2 IRQ Channel */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_Cmd(IN_ADC_DMA_CHAN, ENABLE);
}

void powerbit_set_freq(uint32_t freq)
{
	volatile uint32_t TimerPeriod;
	volatile uint16_t Channel3Pulse;

	TIM_Cmd(PWM_TIM, DISABLE);
	TIM_CtrlPWMOutputs(PWM_TIM, DISABLE);
	TIM_OC3PreloadConfig(PWM_TIM, TIM_OCPreload_Disable);

	// prepocti z frekvence periodu casovace
	TimerPeriod = (SYSTEMCORECLOCK / freq ) - 1;
	Channel3Pulse = (uint16_t) (((uint32_t)PWM_DUTY_INIT * (TimerPeriod - 1)) / PWM_MAX);

	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = PWM_TIM_PRESCALER;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(PWM_TIM, (TIM_TimeBaseInitTypeDef *)&TIM_TimeBaseStructure);

	// OC configuration
	TIM_OCInitStructure.TIM_Pulse = Channel3Pulse;
	TIM_OC3Init(PWM_TIM, (TIM_OCInitTypeDef *)&TIM_OCInitStructure);
	TIM_OC3PreloadConfig(PWM_TIM, TIM_OCPreload_Enable);

	TIM_Cmd(PWM_TIM, ENABLE);
	TIM_CtrlPWMOutputs(PWM_TIM, ENABLE);
}

void powerbit_pwm_init(void)
{
	volatile uint32_t TimerPeriod = 0;
	volatile uint16_t Channel3Pulse = 0;

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
	TimerPeriod = (SYSTEMCORECLOCK / pwmfreq ) - 1;

	/* Compute CCR3 value to generate a duty cycle at PWM_DUTY_INIT%  for channel 3 and 3N */
	Channel3Pulse = (uint16_t) (((uint32_t) PWM_DUTY_INIT * (TimerPeriod - 1)) / PWM_MAX);

	/* PWM_TIM clock enable */
	RCC_APB2PeriphClockCmd(PWM_TIM_CLOCKS , ENABLE);

	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = PWM_TIM_PRESCALER;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(PWM_TIM, (TIM_TimeBaseInitTypeDef *)&TIM_TimeBaseStructure);

	/* Channel 3 Configuration in PWM mode */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = Channel3Pulse;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
	TIM_OC3Init(PWM_TIM, (TIM_OCInitTypeDef *)&TIM_OCInitStructure);

	TIM_OC3PreloadConfig(PWM_TIM, TIM_OCPreload_Enable);

	// nastaveni deadtime generatoru mezi vystupy PWM_TIM_CH3 a PWM_TIM_CH3N
	TIM_DeadTimeStructure.TIM_OSSRState = TIM_OSSRState_Enable;
	TIM_DeadTimeStructure.TIM_OSSIState = TIM_OSSIState_Enable;
	TIM_DeadTimeStructure.TIM_LOCKLevel = TIM_LOCKLevel_1;
	TIM_DeadTimeStructure.TIM_DeadTime = PWM_TIM_DEADTIME;
	TIM_DeadTimeStructure.TIM_Break = TIM_Break_Enable;
	TIM_DeadTimeStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;
	TIM_DeadTimeStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
	TIM_BDTRConfig(PWM_TIM, (TIM_BDTRInitTypeDef *)&TIM_DeadTimeStructure);

	/* PWM_TIM counter enable */
	TIM_Cmd(PWM_TIM, ENABLE);

	/* PWM_TIM Main Output Enable */
	TIM_CtrlPWMOutputs(PWM_TIM, ENABLE);
}

void powerbit_set_duty(uint16_t duty)
{
	volatile uint16_t Channel3Pulse = 0;
	volatile uint16_t TimerPeriod = (SYSTEMCORECLOCK / pwmfreq ) - 1;

	Channel3Pulse = (uint16_t) (((uint32_t) duty * (TimerPeriod - 1)) / PWM_MAX);
	PWM_TIM->CCR3 = Channel3Pulse;
}

void powerbit_set_dutymax(uint16_t duty)
{
	output.dutymax = duty;
}

void powerbit_set_dutymin(uint16_t duty)
{
	output.dutymin = duty;
}

void powerbit_set_const(uint8_t type, int16_t value)
{
	switch (type)
	{
		case 'P':
			regulator.psd_kp = value;
		break;

		case 'I':
		case 'S':
			regulator.psd_ki = value;
		break;

		case 'D':
			regulator.psd_kd = value;
		break;

		default:
		break;
	}
}

void powerbit_wait_for_adc(void)
{
	// Test DMA1 TC flag
	while((DMA_GetFlagStatus(DMA1_FLAG_TC1)) == RESET );

	// Clear DMA TC flag
	DMA_ClearFlag(DMA1_FLAG_TC1);
}

void powerbit_curr_peak(void)
{
	// zkontroluj, zda nedoslo k prekroceni spickove hodnoty proudu
	if (dma_data[CURRENT] >= regulator.currentpeak)
	{
		// pokud ano, sniz stridu
		if (output.duty < PWM_DUTY_STEP)
		{
			output.duty = 0;
		}
		else
		{
			output.duty -= PWM_DUTY_STEP;
		}
		powerbit_set_duty(output.duty);
	}
}

void powerbit_volt_avrg(void)
{
	static volatile uint8_t index = 0;

	volt_average[index++] = dma_data[VOLTAGE];
	if (index >= VOLTAVRGCOUNT)
	{
		index = 0;
	}
}

void powerbit_regulator_process(void)
{
	if (regulator.enabled == TRUE)
	{
		powerbit_psd_calculation();
	}
	else
	{
		output.duty = 0;
		powerbit_set_duty(output.duty);
	}
}

void powerbit_psd_calculation(void)
{
	volatile int16_t psd_intreg;
	volatile int16_t psd_error;
	volatile int16_t psd_value;
	volatile uint16_t currentvoltage;

	powerbit_wait_for_adc();								// pockej na dokonceni mereni

	// MERENI NAPETI
	currentvoltage = powerbit_get_voltage();
	//currentvoltage = (VOLT_MULT * dma_data[VOLTAGE])/VOLT_DIV - VOLT_OFFSET;

	// VYPOCET STAVU REGULATORU
	// aktualni chybova odchylka = pozadovane napeti - soucasne napeti
	psd_error = regulator.req_voltage - currentvoltage;

	// integracni registr = minula hodnota integracniho registru + kI x aktualni chybova odchylka
	psd_intreg = regulator.psd_intreg + psd_error * (regulator.psd_ki/10);

	// omezeni velikosti regulacni odchylky)
	if (psd_intreg > INTREGMAX) {psd_intreg = INTREGMAX;}
	if (psd_intreg < INTREGMIN) {psd_intreg = INTREGMIN;}

	// hodnota regulacniho zasahu = kP x chybova odchylka + integracni registr + kD * rozdil soucasne a minule chybove odchylky
	psd_value = regulator.psd_kp/10 * psd_error + psd_intreg + regulator.psd_kd/10 * (psd_error - regulator.psd_error);

	// akcni zasah podle hodnoty
	volatile int16_t pwm;

	// test prekroceni citlivosti
	if (psd_value > PSDVALUEUPPERLIMIT)
	{
		// musim snizit stridu
		pwm = psd_value - PSDVALUEUPPERLIMIT;

		if (pwm > PSDMAX)
		{
			// hodnota je nadlimitni
			pwm = -output.duty;
		}
		else
		{
			// nyni je pwm = psd_value;
		}
		output.pwmstatus = PWMHIGH;
	}
	else if (psd_value < PSDVALUELOWERLIMIT)
	{
		// zvetsit stridu, nejvetsi strida je PWM_DUTY_MAX
		pwm = psd_value + PSDVALUELOWERLIMIT;
		if (pwm < PSDMIN)
		{
			// hodnota je podlimitni
			pwm = output.dutymax;
		}
		else
		{
			// nyni je pwm = psd_value;
		}
		output.pwmstatus = PWMLOW;
	}
	else
	{
		// bez zasahu
		pwm = 0;
		output.pwmstatus = PWMSTABLE;
	}
	// dorovnej stridu podle pozadovane napocitane zmeny
	pwm += output.duty;
	if (pwm < output.dutymin)
	{
		output.duty = output.dutymin;
	}
	else if (pwm > output.dutymax)
	{
		output.duty = output.dutymax;
	}
	else
	{
		output.duty = pwm;
	}

/*
	// bude pozadovany zasah velky?
	if (psd_value > PSDVALUEUPPERLIMIT)
	{
		// zvysit stridu
		pwm = psd_value - PSDVALUEUPPERLIMIT;

		if (pwm > PSDMAX)
		{
			pwm = PWM_DUTY_MAX;
		}
		else
		{
			//pwm /= 50;
		}
		output.pwmstatus = PWMLOW;
	}
	else if (psd_value < PSDVALUELOWERLIMIT)
	{
		// snizit stridu, nejvetsi strida je PWM_DUTY_MAX
		pwm = psd_value + PSDVALUELOWERLIMIT;
		if (pwm < PSDMIN)
		{
			pwm = -output.duty;
		}
		else
		{
			//pwm /= 50;
		}
		output.pwmstatus = PWMHIGH;
	}
	else
	{
		pwm = 0;
		output.pwmstatus = PWMSTABLE;
	}
	// dorovnej stridu podle pozadovane napocitane zmeny
	pwm += output.duty;
	if (pwm < 0)
	{
		output.duty = 0;
	}
	else
	{
		output.duty = pwm;
	}
	if (output.duty > PWM_DUTY_MAX)
	{
		output.duty = PWM_DUTY_MAX;
	}
*/


	powerbit_set_duty(output.duty);

	// ulozeni hodnoty chybove odchylky pro pristi beh
	regulator.psd_error = psd_error;

	// ulozeni hodnoty integracniho registru pro pristi beh
	regulator.psd_intreg = psd_intreg;

	// ulozeni hodnoty regulatoru pro monitoring
	regulator.psd_value = psd_value;
}

void powerbit_enable(void)
{
	regulator.enabled = TRUE;
}

void powerbit_disable(void)
{
	regulator.enabled = FALSE;
}

void powerbit_set_voltage(uint16_t voltage)
{
	regulator.req_voltage = voltage;
}

uint16_t powerbit_get_voltage(void)
{
	volatile uint8_t i;
	volatile uint32_t sum = 0;

	for (i = 0; i < VOLTAVRGCOUNT; i++)
	{
		sum += volt_average[i];
	}
	sum /= VOLTAVRGCOUNT;
	//return (VOLT_MULT * dma_data[VOLTAGE])/VOLT_DIV - VOLT_OFFSET;
	return (VOLT_MULT * sum)/VOLT_DIV - VOLT_OFFSET;
}

uint16_t powerbit_get_peak(void)
{
	return regulator.currentpeak;
}

powerbit_output_t* powerbit_get_output(void)
{
	return (powerbit_output_t*)&output;
}

void powerbit_set_current(uint16_t current)
{
	regulator.currentpeak = current;
}

/*
#pragma GCC push_options
#pragma GCC optimize ("O0")
#pragma GCC pop_options
*/

