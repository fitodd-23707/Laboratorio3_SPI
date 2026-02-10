/*
 * SPI.c
 *
 * Created: 3/02/2026 00:26:14
 *  Author: Usuario
 */ 

#include "SPI.h"

static void spiReceiveWait(void)
{
	while (!(SPSR & (1 << SPIF)));
}

void spiInit(Spi_Type sType,
Spi_Data_Order sDataOrder,
SPI_CLOCK_Polarity sClockPolarity,
SPI_CLOCK_Phase sClockPhase)
{
	if (sType & (1 << MSTR))   // MASTER
	{
		DDRB |= (1 << DDB3) | (1 << DDB5) | (1 << DDB2); // MOSI, SCK, SS
		DDRB &= ~(1 << DDB4);                           // MISO
		SPCR |= (1 << MSTR);

		uint8_t temp = sType & 0b00000111;

		switch (temp)
		{
			case 0:
			SPCR &= ~((1 << SPR1) | (1 << SPR0));
			SPSR |= (1 << SPI2X);
			break;

			case 1:
			SPCR &= ~((1 << SPR1) | (1 << SPR0));
			SPSR &= ~(1 << SPI2X);
			break;

			case 2:
			SPCR |= (1 << SPR0);
			SPCR &= ~(1 << SPR1);
			SPSR |= (1 << SPI2X);
			break;

			case 3:
			SPCR |= (1 << SPR0);
			SPCR &= ~(1 << SPR1);
			SPSR &= ~(1 << SPI2X);
			break;

			case 4:
			SPCR &= ~(1 << SPR0);
			SPCR |= (1 << SPR1);
			SPSR |= (1 << SPI2X);
			break;

			case 5:
			SPCR &= ~(1 << SPR0);
			SPCR |= (1 << SPR1);
			SPSR &= ~(1 << SPI2X);
			break;

			case 6:
			SPCR |= (1 << SPR0) | (1 << SPR1);
			SPSR &= ~(1 << SPI2X);
			break;
		}
	}
	else   // SLAVE
	{
		DDRB |= (1 << DDB4);  // MISO
		DDRB &= ~((1 << DDB3) | (1 << DDB5) | (1 << DDB2)); // MOSI, SCK, SS
		SPCR &= ~(1 << MSTR);
	}

	SPCR |= (1 << SPE) | sDataOrder | sClockPolarity | sClockPhase;
}

void spiWrite(uint8_t dat)
{
	SPDR = dat;
}

unsigned spiDataReady(void)
{
	return (SPSR & (1 << SPIF)) ? 1 : 0;
}

uint8_t spiRead(void)
{
	spiReceiveWait();
	return SPDR;
}
