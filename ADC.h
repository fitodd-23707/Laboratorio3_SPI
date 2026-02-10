/*
 * ADC.h
 *
 * Created: 1/02/2026 18:12:23
 *  Author: Usuario
 */ 

#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>
#include <stdint.h>

void ADC_Init(void);
uint16_t ADC_Read(uint8_t channel);

#endif