#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "ADC/ADC.h"
#include "SPI/SPI.h"

uint8_t comando = '3';
uint8_t index = 0;
uint8_t num = 0;

float V = 0.0;
char txt[5] = "0.00";

/* ===== ISR SPI ===== */
ISR(SPI_STC_vect)
{
	uint8_t rx = SPDR;   // byte recibido del maestro

	if (rx == '1')              // pedir A0
	{
		comando = '1';
		index = 0;
	}
	else if (rx == '2')         // pedir A3
	{
		comando = '2';
		index = 0;
	}
	else if (rx == '0')         // dummy ? enviar dato
	{
		SPDR = txt[index];      // PRE-cargar dato
		index++;
		if (index >= 5) index = 0;
	}
	else{
		comando = 'n';
		num =  rx;
	}
}

void MostrarBinario(uint8_t value){
	
	if (value & (1 << 0)) PORTB |=  (1 << PORTB1);
	else              PORTB &= ~(1 << PORTB1);

	if (value & (1 << 1)) PORTB |=  (1 << PORTB0);
	else              PORTB &= ~(1 << PORTB0);
	
	if (value & (1 << 2)) PORTD |=  (1 << PORTD7);
	else              PORTD &= ~(1 << PORTD7);

	if (value & (1 << 3)) PORTD |=  (1 << PORTD6);
	else              PORTD &= ~(1 << PORTD6);
	
	if (value & (1 << 4)) PORTD |=  (1 << PORTD5);
	else              PORTD &= ~(1 << PORTD5);

	if (value & (1 << 5)) PORTD |=  (1 << PORTD4);
	else               PORTD &= ~(1 << PORTD4);
	
	if (value & (1 << 6)) PORTD |=  (1 << PORTD3);
	else               PORTD &= ~(1 << PORTD3);

	if (value & (1 << 7)) PORTD |=  (1 << PORTD2);
	else                PORTD &= ~(1 << PORTD2);
}

int main(void)
{
	cli();

	/* SPI SLAVE */
	spiInit(SPI_MASTER_OSC_SS,
	SPI_DATA_ORDER_MSB,
	SPI_CLOCK_IDLE_LOW,
	SPI_CLOCK_FIRST_EDGE);

	SPCR |= (1 << SPIE);   // habilitar interrupción SPI

	ADC_Init();

	sei();

	while (1)
	{
		if (comando == '1' || comando == '2')
		{
			uint16_t adc;

			if (comando == '1')
			adc = ADC_Read(0);   // A0
			else
			adc = ADC_Read(1);   // A3

			V = (adc * 5.0f) / 1023.0f;

			uint16_t v = (uint16_t)(V * 100 + 0.5f);

			txt[0] = '0' + (v / 100);        // 0–5
			txt[1] = '.';
			txt[2] = '0' + (v / 10) % 10;
			txt[3] = '0' + (v % 10);
			txt[4] = '\0';

			index = 0;        //  IMPORTANTE
			comando = '3';    // listo para enviar
		}
		
		if (comando == 'n'){
			MostrarBinario(num);
		}
	}

}