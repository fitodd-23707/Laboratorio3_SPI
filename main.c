#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdint.h>

#include "SPI.h"
#include "UART.h"

#define RX_BUF_LEN 4

volatile char rxbuf[RX_BUF_LEN];
volatile uint8_t rx_i = 0;
volatile uint8_t value_ready = 0;
volatile uint8_t rx_value = 0;

char buffer[7];   // 6 chars + '\0'

uint8_t index = 0;

ISR(USART_RX_vect){
	
	char c = UDR0;  // leer byte recibido (limpia RXC)

	// Ignorar CR y LF como terminadores
	if (c == '\r' || c == '\n')
	{
		// Si no se escribió nada, ignora
		if (rx_i == 0) {
			return;
		}

		// Terminar el string
		rxbuf[rx_i] = '\0';

		// Convertir a número (base 10)
		char *endptr;
		unsigned long v = strtoul((const char*)rxbuf, &endptr, 10);

		// Validación: todo debe ser dígitos y rango 0..255
		if (*endptr == '\0' && v <= 255UL)
		{
			rx_value = (uint8_t)v;
			value_ready = 1;     // bandera para que el main aplique a LEDs
		}

		// Preparar para la próxima entrada
		rx_i = 0;
		return;
	}

	// Aceptar solo dígitos
	if (c < '0' || c > '9') {
		rx_i = 0;
		return;
	}

	// Guardar dígito si hay espacio (máximo 3 dígitos)
	if (rx_i < (RX_BUF_LEN - 1))
	{
		rxbuf[rx_i++] = c;
	}
	else
	{
		// Overflow: demasiados dígitos -> reiniciar
		rx_i = 0;
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

void setup(void)
{
	cli();

	UART_init();
	_delay_ms(200);
	UART_Write_str("Maestro SPI iniciado\r\n");

	SPI_init(SPI_MASTER_OSC_DIV4, SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOCK_FIRST_EDGE);

	PORTB |= (1 << PORTB2);
	
	DDRB |= (1 << PORTB0) | (1 << PORTB1);
	DDRD |= (1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7);

	sei();
}

int main(void)
{
	setup();

	while (1)
	{
		uint8_t dummy;
				
		if (value_ready){
			value_ready = 0;
			MostrarBinario(rx_value);
			
			UCSR0B &= ~(1 << RXCIE0);

			/* HABLAR PARA ENVIAR NÚMERO */
			PORTB &= ~(1 << PORTB2);   // SS = 0

			SPI_Write(rx_value);            // Pedir A0
			dummy = SPI_Read();        // IMPORTANTE: leer y DESCARTAR respuesta del byte de comando

			PORTB |= (1 << PORTB2);    // SS = 1
			
			UCSR0B |= (1 << RXCIE0);
		}
		
		UCSR0B &= ~(1 << RXCIE0);

		/* ========= POTENCIOMETRO A0 ========= */
		PORTB &= ~(1 << PORTB2);   // SS = 0

		SPI_Write('1');            // Pedir A0
		dummy = SPI_Read();        // IMPORTANTE: leer y DESCARTAR respuesta del byte de comando
		//UART_Write_char(dummy);

		_delay_us(10);

		for (uint8_t i = 0; i < 5; i++)
		{
			SPI_Write('0');                 // Dummy para generar clocks (o 0x00)
			buffer[i] = (char)SPI_Read();   // Byte recibido del esclavo
			_delay_us(10);
		}

		PORTB |= (1 << PORTB2);    // SS = 1
		
		UCSR0B |= (1 << RXCIE0);

		buffer[6] = '\0';          // Terminar string para UART_Write_str

		UART_Write_str("P1: ");
		UART_Write_char(buffer[1]);
		UART_Write_char(buffer[2]);
		UART_Write_char(buffer[3]);
		UART_Write_char(buffer[4]);
		//UART_Write_char(buffer[5]);
		//UART_Write_char(buffer[6]);
		UART_Write_str(" V\r\n");

		_delay_ms(100);
		
		UCSR0B &= ~(1 << RXCIE0);

		/* ========= POTENCIOMETRO A3 ========= */
		PORTB &= ~(1 << PORTB2);   // SS = 0

		SPI_Write('2');            // Pedir A3
		dummy = SPI_Read();        // Leer y DESCARTAR respuesta del byte de comando

		_delay_us(10);

		for (uint8_t i = 0; i < 5; i++)
		{
			SPI_Write('0');                 // Dummy para generar clocks (o 0x00)
			buffer[i] = (char)SPI_Read();   // Byte recibido del esclavo
			_delay_us(10);
		}

		PORTB |= (1 << PORTB2);    // SS = 1
		
		UCSR0B |= (1 << RXCIE0);


		buffer[6] = '\0';

		UART_Write_str("P2: ");
		UART_Write_char(buffer[1]);
		UART_Write_char(buffer[2]);
		UART_Write_char(buffer[3]);
		UART_Write_char(buffer[4]);
		//UART_Write_char(buffer[5]);
		//UART_Write_char(buffer[6]);
		UART_Write_str(" V\r\n");

		UART_Write_str("------------------\r\n");

		_delay_ms(500);
	}
}