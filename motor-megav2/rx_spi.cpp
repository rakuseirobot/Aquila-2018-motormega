/********************************************************************************/
/*--
	@file			rx_spi.c
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
#include	"rx_spi.hpp"

/* Variables -----------------------------------------------------------------*/

uint8_t	data;
uint8_t flag;
uint8_t setdata;

/* Functions -----------------------------------------------------------------*/
/**
  * @brief	Functions to configure SPI
  * @param  None
  * @retval None
  **/
void	SPI_Slave_Config(void)
{
	DDRB = (1<<PORTB4);
	SPCR = _BW(SPE)	|	_BW(SPIE);
	data = flag = 0;
	sei();
}

/**
  * @brief	Functions to confirm of receipt status
  * @param  None
  * @retval 0 : Has not been received.
			1 : Has been received
  **/
uint8_t	SPI_ReceiveFlag(void)
{
	return flag;
}

/**
  * @brief	Functions to read receipt data
  * @param  None
  * @retval Received data
  **/
uint8_t SPI_ReceiveData(void)
{
	flag=0;
	return data;
}

/**
  * @brief	Functions to set of data that will be sent to the Master.
  * @param  data	data that will be sent to the Master.
  * @retval None
  **/
void	SPI_SetSendData(uint8_t data)
{
	cli();
	SPDR = setdata = data;
	sei();
}

//Interrupt function
ISR(SPI_STC_vect)
{
	data = SPDR;
	if(data!=0x00){
		flag = 1;
		SPI_SetSendData(2);
	}
	else{
		SPDR = setdata;
	}
}
