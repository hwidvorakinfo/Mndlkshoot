/*
 * commands.c
 *
 *  Created on: 7. 5. 2019
 *      Author: daymoon
 */

#include "commands.h"
#include "string.h"

// prikazy
static const uint8_t COMMAND_ON[] = "ON";
static volatile command_t command_on = {(uint8_t *)&COMMAND_ON, &cmd_powerbit_on, TRUE};			// prikaz ON, zapni se
static const uint8_t COMMAND_OFF[] = "OFF";
static volatile command_t command_off = {(uint8_t *)&COMMAND_OFF, &cmd_powerbit_off, TRUE};		// prikaz OFF, vypni se
static const uint8_t COMMAND_VOLT[] = "VOLT**.**";
static volatile command_t command_volt = {(uint8_t *)&COMMAND_VOLT, &cmd_powerbit_volt, TRUE};	// prikaz VOLT**.*, nastav vystup
static const uint8_t COMMAND_CURR[] = "CURR****";
static volatile command_t command_curr = {(uint8_t *)&COMMAND_CURR, &cmd_powerbit_curr, TRUE};	// prikaz CURR****, nastav maximalni proud
static const uint8_t COMMAND_STAT[] = "STAT";
static volatile command_t command_stat = {(uint8_t *)&COMMAND_STAT, &cmd_powerbit_stat, TRUE};	// prikaz STAT, stav zdroje
static const uint8_t COMMAND_FREQ[] = "FREQ******";
static volatile command_t command_freq = {(uint8_t *)&COMMAND_FREQ, &cmd_powerbit_freq, TRUE};	// prikaz FREQ******, nastav frekvenci pwm
static const uint8_t COMMAND_MAX[] = "MAX**";
static volatile command_t command_max = {(uint8_t *)&COMMAND_MAX, &cmd_powerbit_dutymax, TRUE};	// prikaz MAX**, nastav maximalni povolenou duty
static const uint8_t COMMAND_MIN[] = "MIN**";
static volatile command_t command_min = {(uint8_t *)&COMMAND_MIN, &cmd_powerbit_dutymin, TRUE};	// prikaz MIN**, nastav minimalni povolenou duty
static const uint8_t COMMAND_KX[] = "K****.*";
static volatile command_t command_kx = {(uint8_t *)&COMMAND_KX, &cmd_powerbit_const, TRUE};		// prikaz K****.*, nastav danou konstantu zesileni


uint8_t commands_parse(uint8_t *pattern, uint8_t *command);
uint8_t cmd_isnumber(uint8_t *character);

void commands_process(void)
{
	COMMAND_STATUS (*p_func)();

	// parsing prikazu ON
	if (commands_parse(command_on.command, get_Rx_buffer()))
	{
		if (command_on.enabled)
		{
			// zavolani obsluzne funkce prikazu
			p_func = command_on.p_itemfunc;
			p_func(NULL);
		}
		//return;
	}
	// parsing prikazu OFF
	else if (commands_parse(command_off.command, get_Rx_buffer()))
	{
		if (command_off.enabled)
		{
			// zavolani obsluzne funkce prikazu
			p_func = command_off.p_itemfunc;
			p_func(NULL);
		}
		//return;
	}
	// parsing prikazu VOLT**.*
	else if (commands_parse(command_volt.command, get_Rx_buffer()))
	{
		if (command_volt.enabled)
		{
			// zavolani obsluzne funkce prikazu
			p_func = command_volt.p_itemfunc;
			p_func(get_Rx_buffer());
		}
		//return;
	}
	// parsing prikazu CURR****
	else if (commands_parse(command_curr.command, get_Rx_buffer()))
	{
		if (command_curr.enabled)
		{
			// zavolani obsluzne funkce prikazu
			p_func = command_curr.p_itemfunc;
			p_func(get_Rx_buffer());
		}
		//return;
	}
	// parsing prikazu STAT
	else if (commands_parse(command_stat.command, get_Rx_buffer()))
	{
		if (command_stat.enabled)
		{
			// zavolani obsluzne funkce prikazu
			p_func = command_stat.p_itemfunc;
			p_func(get_Rx_buffer());
		}
		//return;
	}
	// parsing prikazu FREQ*******
	else if (commands_parse(command_freq.command, get_Rx_buffer()))
	{
		if (command_freq.enabled)
		{
			// zavolani obsluzne funkce prikazu
			p_func = command_freq.p_itemfunc;
			p_func(get_Rx_buffer());
		}
		//return;
	}
	// parsing prikazu MAX**
	else if (commands_parse(command_max.command, get_Rx_buffer()))
	{
		if (command_max.enabled)
		{
			// zavolani obsluzne funkce prikazu
			p_func = command_max.p_itemfunc;
			p_func(get_Rx_buffer());
		}
		//return;
	}
	// parsing prikazu MIN**
	else if (commands_parse(command_min.command, get_Rx_buffer()))
	{
		if (command_min.enabled)
		{
			// zavolani obsluzne funkce prikazu
			p_func = command_min.p_itemfunc;
			p_func(get_Rx_buffer());
		}
		//return;
	}
	// parsing prikazu K****.*
	else if (commands_parse(command_kx.command, get_Rx_buffer()))
	{
		if (command_kx.enabled)
		{
			// zavolani obsluzne funkce prikazu
			p_func = command_kx.p_itemfunc;
			p_func(get_Rx_buffer());
		}
		//return;
	}
	else
	{
		// zbyva jiz jen posledni varianta - spatny prikaz
		commands_wrong_cmd();
	}
}


uint8_t commands_parse(uint8_t *pattern, uint8_t *command)
{
	uint8_t i = 0;
	uint8_t retval = TRUE;

	// porovnani prichoziho prikazu se vzorem. Hvezdicka je zastupny znak pro jedinou cifru cisla.
	while(pattern[i] != 0)
	{
		if (pattern[i] == '*')
		{
			i++;
			continue;
		}
		else if (pattern[i] != command[i])
		{
			retval = FALSE;
			return retval;
		}
		else {
			i++;
			continue;
		}
	}

	usart_send_text((uint8_t *)command);
	usart_newline();

	return retval;
}

void commands_wrong_cmd(void)
{
	uint8_t text[] = "ER";

	usart_send_text((uint8_t *)&text);
	usart_newline();
}

void commands_ok_cmd(void)
{
	uint8_t text[] = "OK";

	usart_send_text((uint8_t *)&text);
	usart_newline();
}

// handlery prikazu
// prikaz ON
COMMAND_STATUS cmd_powerbit_on(void *p_i)
{
	powerbit_enable();

	commands_ok_cmd();

	return COMMANDOK;
}

// prikaz OFF
COMMAND_STATUS cmd_powerbit_off(void *p_i)
{
	powerbit_disable();

	commands_ok_cmd();

	return COMMANDOK;
}

// prikaz VOLT**.*
COMMAND_STATUS cmd_powerbit_volt(void *p_i)
{
	// tvar prikazu je VOLT**.*, kde **.* je napeti ve tvaru 10.5 ve Voltech
	COMMAND_STATUS retval = COMMANDOK;
	uint16_t voltage;
	uint8_t *p_int = (uint8_t *)p_i;

#define VOLTOFFSET		4

	// kontrola dat v retezci napeti
	if ((cmd_isnumber(&p_int[VOLTOFFSET])) && (cmd_isnumber(&p_int[VOLTOFFSET+1])) && (cmd_isnumber(&p_int[VOLTOFFSET+3])) && (cmd_isnumber(&p_int[VOLTOFFSET+4])))
	{
		voltage = 1000 * (p_int[VOLTOFFSET] - '0') + 100 * (p_int[VOLTOFFSET+1] - '0') + 10 * (p_int[VOLTOFFSET+3] - '0') + (p_int[VOLTOFFSET+4] - '0');
		if (voltage > VOLTAGEMAX)
		{
			// napeti je prilis velke
			retval = COMMANDWRONG;
			commands_wrong_cmd();
			return retval;
		}
	}
	else
	{
		// spatne sestaveny prikaz
		retval = COMMANDWRONG;
		commands_wrong_cmd();
		return retval;
	}

	// napeti je spravne a muze byt nastaveno
	powerbit_set_voltage(voltage);

	commands_ok_cmd();

	return COMMANDOK;
#undef VOLTOFFSET
}

// prikaz CURR****
COMMAND_STATUS cmd_powerbit_curr(void *p_i)
{
	// tvar prikazu je CURR****, kde **** je maximalni proud ve tvaru **** v ADC hodnotach
	COMMAND_STATUS retval = COMMANDOK;
	uint16_t current;
	uint8_t *p_int = (uint8_t *)p_i;

#define CURROFFSET		4

	// kontrola dat v retezci napeti
	if ((cmd_isnumber(&p_int[CURROFFSET])) && (cmd_isnumber(&p_int[CURROFFSET+1])) && (cmd_isnumber(&p_int[CURROFFSET+2])) && (cmd_isnumber(&p_int[CURROFFSET+3])))
	{
		current = 1000 * (p_int[CURROFFSET] - '0') + 100 * (p_int[CURROFFSET+1] - '0') + 10 * (p_int[CURROFFSET+2] - '0') + (p_int[CURROFFSET+3] - '0');
		if (current > CURRENTMAX)
		{
			// proud je prilis velky
			retval = COMMANDWRONG;
			commands_wrong_cmd();
			return retval;
		}
	}
	else
	{
		// spatne sestaveny prikaz
		retval = COMMANDWRONG;
		commands_wrong_cmd();
		return retval;
	}

	// proud je spravny a muze byt nastaven
	powerbit_set_current(current);

	commands_ok_cmd();

	return COMMANDOK;

#undef CURROFFSET
}

// prikaz STAT
COMMAND_STATUS cmd_powerbit_stat(void *p_i)
{
	uint8_t text[] = "VOLT: 00.00V, PEAK: 1000, PWM: 99%";
	uint16_t value;

	powerbit_wait_for_adc();								// pockej na dokonceni mereni

	// vypis aktualni napeti
#define VOLTVALUEOFFSET		6
#define PEAKVALUEOFFSET		20
#define DUTYOFFSET			31
#define PWMOFFSET			35


	value = powerbit_get_voltage();
	text[VOLTVALUEOFFSET] = value / 1000 + '0';
	value %= 1000;
	text[VOLTVALUEOFFSET+1] = value / 100 + '0';
	value %= 100;
	text[VOLTVALUEOFFSET+3] = value / 10 + '0';
	value %= 10;
	text[VOLTVALUEOFFSET+4] = value + '0';

	// vypis peak current
	value = powerbit_get_peak();
	text[PEAKVALUEOFFSET] = value / 1000 + '0';
	value %= 1000;
	text[PEAKVALUEOFFSET+1] = value / 100 + '0';
	value %= 100;
	text[PEAKVALUEOFFSET+2] = value / 10 + '0';
	value %= 10;
	text[PEAKVALUEOFFSET+3] = value + '0';

	// vypis duty
	value = powerbit_get_output()->duty;
	value /= 100;
	text[DUTYOFFSET] = value / 10 + '0';
	value %= 10;
	text[DUTYOFFSET+1] = value + '0';
	text[DUTYOFFSET+3] = 0;

/*
	// vypis stav PWM
	value = powerbit_get_output->pwmstatus();
	switch (value)
	{
		case PWMSTABLE:
			text[PWMOFFSET] = 'S';
			text[PWMOFFSET+1] = 'T';
			text[PWMOFFSET+2] = 'A';
			text[PWMOFFSET+3] = 'B';
			text[PWMOFFSET+4] = 'L';
			text[PWMOFFSET+5] = 'E';
			text[PWMOFFSET+6] = 0;
		break;

		case PWMLOW:
			text[PWMOFFSET] = 'L';
			text[PWMOFFSET+1] = 'O';
			text[PWMOFFSET+2] = 'W';
			text[PWMOFFSET+3] = 0;
		break;

		case PWMHIGH:
			text[PWMOFFSET] = 'H';
			text[PWMOFFSET+1] = 'I';
			text[PWMOFFSET+2] = 'G';
			text[PWMOFFSET+3] = 'H';
			text[PWMOFFSET+4] = 0;
		break;

		default:
			text[PWMOFFSET] = '-';
			text[PWMOFFSET+1] = '-';
			text[PWMOFFSET+2] = '-';
			text[PWMOFFSET+3] = 0;
		break;
	}*/

	usart_send_text((uint8_t *)&text);
	usart_newline();

	commands_ok_cmd();

	return COMMANDOK;

#undef VOLTVALUEOFFSET
#undef PEAKVALUEOFFSET
#undef DUTYOFFSET
#undef PWMOFFSET

}


// prikaz FREQ******
COMMAND_STATUS cmd_powerbit_freq(void *p_i)
{
	// tvar prikazu je FREQ******, kde ****** je kmitocet v Hz
	COMMAND_STATUS retval = COMMANDOK;
	uint32_t freq;
	uint8_t *p_int = (uint8_t *)p_i;

#define FREQOFFSET		4

	// kontrola dat v retezci napeti
	if ((cmd_isnumber(&p_int[FREQOFFSET])) && (cmd_isnumber(&p_int[FREQOFFSET+1])) && (cmd_isnumber(&p_int[FREQOFFSET+2])) && (cmd_isnumber(&p_int[FREQOFFSET+3]) && (cmd_isnumber(&p_int[FREQOFFSET+4])) && (cmd_isnumber(&p_int[FREQOFFSET+5]))))
	{
		freq = 100000 * (p_int[FREQOFFSET] - '0') + 10000 * (p_int[FREQOFFSET+1] - '0') + 1000 * (p_int[FREQOFFSET+2] - '0') + 100*(p_int[FREQOFFSET+3] - '0') + 10*(p_int[FREQOFFSET+4] - '0') + (p_int[FREQOFFSET+5] - '0');
		if (freq > PWMFREQMAX)
		{
			// frekvence je prilis velka
			retval = COMMANDWRONG;
			commands_wrong_cmd();
			return retval;
		}
	}
	else
	{
		// spatne sestaveny prikaz
		retval = COMMANDWRONG;
		commands_wrong_cmd();
		return retval;
	}

	// frekvence je spravna a muze byt nastavena
	powerbit_set_freq(freq);

	commands_ok_cmd();

	return COMMANDOK;
#undef FREQOFFSET
}

// prikaz MAX**
COMMAND_STATUS cmd_powerbit_dutymax(void *p_i)
{
	// tvar prikazu je MAX**, kde ** je maximalni duty
	COMMAND_STATUS retval = COMMANDOK;
	uint8_t duty;
	uint8_t *p_int = (uint8_t *)p_i;

#define DUTYOFFSET		3

	// kontrola dat v retezci napeti
	if ((cmd_isnumber(&p_int[DUTYOFFSET])) && (cmd_isnumber(&p_int[DUTYOFFSET+1])))
	{
		duty = 10 * (p_int[DUTYOFFSET] - '0') + (p_int[DUTYOFFSET+1] - '0');
		if ((duty > PWM_DUTY_MAX/100) || (duty < powerbit_get_output()->dutymin))
		{
			// duty je prilis velka
			retval = COMMANDWRONG;
			commands_wrong_cmd();
			return retval;
		}
	}
	else
	{
		// spatne sestaveny prikaz
		retval = COMMANDWRONG;
		commands_wrong_cmd();
		return retval;
	}

	// frekvence je spravna a muze byt nastavena
	powerbit_set_dutymax(100*duty);

	commands_ok_cmd();

	return COMMANDOK;
#undef DUTYOFFSET
}

// prikaz MIN**
COMMAND_STATUS cmd_powerbit_dutymin(void *p_i)
{
	// tvar prikazu je MAX**, kde ** je maximalni duty
	COMMAND_STATUS retval = COMMANDOK;
	uint8_t duty;
	uint8_t *p_int = (uint8_t *)p_i;

#define DUTYOFFSET		3

	// kontrola dat v retezci napeti
	if ((cmd_isnumber(&p_int[DUTYOFFSET])) && (cmd_isnumber(&p_int[DUTYOFFSET+1])))
	{
		duty = 10 * (p_int[DUTYOFFSET] - '0') + (p_int[DUTYOFFSET+1] - '0');
		if (duty > powerbit_get_output()->dutymax)
		{
			// duty je prilis velka
			retval = COMMANDWRONG;
			commands_wrong_cmd();
			return retval;
		}
	}
	else
	{
		// spatne sestaveny prikaz
		retval = COMMANDWRONG;
		commands_wrong_cmd();
		return retval;
	}

	// frekvence je spravna a muze byt nastavena
	powerbit_set_dutymin(100*duty);

	commands_ok_cmd();

	return COMMANDOK;
}

// prikaz K****.*
COMMAND_STATUS cmd_powerbit_const(void *p_i)
{
	// tvar prikazu je K****.*, kde tvar je KP+NN.N a dalsi pro dalsi zesileni
	COMMAND_STATUS retval = COMMANDOK;
	int16_t value;
	uint8_t *p_int = (uint8_t *)p_i;

#define CONSTTYPE		1
#define CONSTSIGN		2
#define CONSTNUM		3

	// urceni hodnoty
	if ((cmd_isnumber(&p_int[CONSTNUM])) && (cmd_isnumber(&p_int[CONSTNUM+1])) && (cmd_isnumber(&p_int[CONSTNUM+3])))
	{
		value = 100*(p_int[CONSTNUM] - '0') + 10*(p_int[CONSTNUM+1] - '0') + (p_int[CONSTNUM+3] - '0');
		if (p_int[CONSTSIGN] == '-')
		{
			value *= -1;
		}
		else if (p_int[CONSTSIGN] != '+')
		{
			// spatne sestaveny prikaz
			retval = COMMANDWRONG;
			commands_wrong_cmd();
			return retval;
		}
	}
	else
	{
		// spatne sestaveny prikaz
		retval = COMMANDWRONG;
		commands_wrong_cmd();
		return retval;
	}

	// urceni typu konstanty
	switch (p_int[CONSTTYPE])
	{
		case 'P':
			powerbit_set_const('P', value);
		break;

		case 'I':
		case 'S':
			powerbit_set_const('I', value);
		break;
		case 'D':
			powerbit_set_const('D', value);
		break;
		default:
			// spatne sestaveny prikaz
			retval = COMMANDWRONG;
			commands_wrong_cmd();
			return retval;
		break;
	}
	commands_ok_cmd();

	return COMMANDOK;
#undef CONSTTYPE
#undef CONSTSIGN
#undef CONSTNUM
}

uint8_t cmd_isnumber(uint8_t *character)
{
	if ((*character >= '0') && (*character <= '9'))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
