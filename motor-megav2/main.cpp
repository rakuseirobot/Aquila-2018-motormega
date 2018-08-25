/*
 * motor-mega.c
 *
 * Created: 2017/08/22 17:45:23
 * Author : shun2
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "transmission.hpp"
#include "rx_spi.hpp"

//#include "motorv2.hpp"
#include "motorv7.hpp"
//#include "motor.hpp"

/******************************************************************************
PC2:CS
PC3:INB
PC4:INA
PC5:EN
PD6:PWM
PD2:RO

PWM-âÒì]êî
30-33
50-52 10âÒì]Å@0.2s
100-90
120-98
*********************************************************************************/
//static long int val;//	-2,147,483,648 ? 2,147,483,647
int main(void)
{
	DDRD=0b00000010;
	SPI_Slave_Config();
	init_motor();
	init_uart(38400);
	_delay_ms(50);
	/* Replace with your application code */
	SPI_SetSendData(1);
	uart_putstr("wakeup!\n\r");
	sei();
	while(1){
		//advance();
		//OCR0A=0x5F;
		//_delay_ms(3000);
		//reverce();
		//_delay_ms(3000);
		//reverce();
		//OCR0A=0xFF;
		//uart_putdec(no);
		//uart_putstr("\n\r");
		//while(!SPI_ReceiveFlag());
			 //uint8_t ji = SPI_ReceiveData();
			 //uart_putdec(ji);
			 //uart_putstr("\n\r");
		//uart_putstr("motor");
			motor();	
		//uart_putdec(tempro);
		//uart_putstr("--1process Finisfhed--\n\r");
		//motor_3byte_test();
	}
	return(0);
}

//int main(void){
	//init_motor();
	//init_uart(38400);
	//_delay_ms(50);
	//count_start();
	//while(1){
		//if(flag==1){
			//count_stop();
			//uart_putstr("over");
			//uart_putstr("\n\r");
			//count_start();
		//}
		//uart_putdec(TCNT1);
		//uart_putstr("\n\r");
		//
	//}
//}

