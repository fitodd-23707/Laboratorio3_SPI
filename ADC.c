/*
 * ADC.c
 *
 * Created: 1/02/2026 18:12:02
 *  Author: Usuario
 */ 

#include "ADC.h"

void ADC_Init(void)
{
	/* Referencia AVcc = 5V */
	ADMUX = (1 << REFS0);

	/* Habilita ADC – Prescaler 128 */
	ADCSRA = (1 << ADEN) |
	(1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_Read(uint8_t channel)
{
	/* Selecciona canal (0–7) */
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

	/* Inicia conversión */
	ADCSRA |= (1 << ADSC);

	/* Espera fin */
	while (ADCSRA & (1 << ADSC));

	return ADC;
}