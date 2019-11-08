/*
 * powerbit_types.h
 *
 *  Created on: 14. 5. 2019
 *      Author: daymoon
 */

#ifndef POWERBIT_TYPES_H_
#define POWERBIT_TYPES_H_


typedef enum {
	OUTPUT0 = 0,
	OUTPUT1,
} OUTPUT_INDEX;

typedef enum {
	PHASE0 = 0,
	PHASE1,
} PHASE;

typedef enum {
	INIT = 0,
	PWMSTABLE,
	PWMLOW,
	PWMHIGH
} PWM_STATUS;

typedef struct {
	uint16_t req_voltage;
	int16_t psd_kp;
	int16_t psd_ki;
	int16_t psd_kd;
	int16_t psd_intreg;
	int16_t psd_error;
	int16_t psd_value;
	uint16_t currentpeak;
	uint8_t enabled;
} powerbit_regulator_t;

typedef struct {
	const OUTPUT_INDEX index;
	const GPIO_TypeDef * port;
	const uint16_t pin;
	const PHASE phase;
	uint16_t duty;
	uint16_t dutymin;
	uint16_t dutymax;
	PWM_STATUS pwmstatus;
} powerbit_output_t;


#endif /* POWERBIT_TYPES_H_ */
