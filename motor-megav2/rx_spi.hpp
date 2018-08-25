/********************************************************************************/
/*--
	@file			rx_spi.h
	@author			Radiandat
	@version		1.00a
	@date			2013/07/25
	@brief			This is a file to control SPI Slave(Core : ATmega644P)

	@section HISTORY
		2011/04/28	1.00a	start here.
		2013/07/25	1.10a

	Copyright(C) 2013 Tomoaki Fukuda all rights reserved.
--*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include	<avr/io.h>
#include	<avr/interrupt.h>

#define	_BW(x)	(1<<x)

/* Functions Prototypes ------------------------------------------------------*/
void	SPI_Slave_Config(void);
uint8_t	SPI_ReceiveFlag(void);
uint8_t SPI_ReceiveData(void);
void	SPI_SetSendData(uint8_t data);
