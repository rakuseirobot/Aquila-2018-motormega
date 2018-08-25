/*
 * motor.h
 *
 * Created: 2017/08/23 9:06:04
 *  Author: shun2
 */ 


#ifndef MOTOR_H_
#define MOTOR_H_
/******************************************************************************
PC2:CS
PC3:INB
PC4:INA
PC5:EN
PD6:PWM
PD2:RO
*********************************************************************************/
/************************************************
PORTB |= _BV(PB1); �@�@//PB1�s��������High�ɂ��A�c��͕ω������Ȃ�
PORTB &= ~_BV(PB3); �@�@//PB3�s��������Low�ɂ��A�c��͕ω������Ȃ�


0.0000512��1024�����̈�J�E���g = 0.0512ms
TCNT1�̏���l=65535
�ő�J�E���g�b�� 3.355392s
20RPM�z�� 1�K�^�K�^ = 0.0003571428571s
10�K�^�K�^=0.003571428571��70�J�E���g(1024����)

************************************************/
//////////////////////////Define�B/////////////////////////////
#define CS (1<<PORTC2)
#define INB (1<<PORTC3)
#define INA (1<<PORTC4)
#define EN (1<<PORTC5)

#include "rx_spi.hpp"

#define rotation 8400
///////////////////////////////////////////�O���[�o���錾////////////////////////////////////////////////////
void init_motor(void); //initiarize
void advance(void); //�O�i
void reverce(void); //��i
void brake(void); //�u���[�L
void move(int i); //����(1:�O�i,2:��i�@����ȊO:�u���[�L)
int motor(void);//SPI�ǂ݁����[�^�[������
void count_start(void);
void count_stop(void);
void motor_3byte_test(void);
int motor_distance(void);

//////////////////////////////////////////�O���[�o���ϐ�////////////////////////////////////////////////////
int best = 0;
uint16_t timr = 0;
uint8_t speed = 0;//�X�s�[�h�ϐ�
unsigned int cot = 0; //�ꎞ��]���p�ϐ�
unsigned int no=0;
int ro = 10; //�^�C�}�[��r�@��]���������l
float timbest = 3900; //�^�C�}�[��r�@�x�X�g�l
float timcnt = 3900;
uint8_t noflag=0;
uint16_t count = 0;
//long int distance[8]={1500,rotation/3,rotation/4,0,rotation};
//uint16_t distance[7] = {0,47,94,29,0,0,0} ; //1�}�X,2�}�X,���V�n --�㏸�[�A���~�[
uint16_t distance[7] = {0,100,186,63,0,0,0};//1�}�X,2�}�X,���V�n --���[���荞��
void init_motor(void){
	DDRC |= CS|INB|INA|EN;
	DDRD |= (0<<PIND2)|(1<<PORTD6);
	PORTD |= (1<<PORTD2);
	EICRA = 0b00000001;
	EIFR  = 0b00000001;
	EIMSK = 0b00000001;
	TCCR0A = 0b10000011;
	TCCR0B = 0b00000011;
}

void advance(void){
	PORTC = INB|EN;
}
void reverce(void){
	PORTC = INA|EN;
}
void brake(void){
	PORTC = INB|INA|EN;
	count_stop();
}
void move(int i){
	if(i==1){
		advance();
	}
	else if(i==2){
		reverce();
	}
	else brake();
}
int temp = 0;
int motor(void){
	no=0;
	cot=0;
	timr=0;
	while(!SPI_ReceiveFlag());
	uint8_t num = SPI_ReceiveData();

	SPI_SetSendData(2);
	
	uint8_t fb = num>>6;
	speed = (num>>3)&0b00000111;
	uint8_t dis = num&0b00000111;
	uart_putstr("dir");
	uart_putdec(fb);
	uart_putdec(speed);
	uart_putdec(dis);
	uart_putstr("\n\r");
	best = speed/4;
	//timbest = timcnt*5/speed;
	
	if(fb==3 || speed==0){
		brake();
		SPI_SetSendData(1);
		temp= 1;
	}
	else if(fb==1){
		advance();
	}
	else if(fb==2){
		reverce();
	}
	if(!temp){
		//OCR0A = (speed*10) - 8;
		OCR0A = speed * 20;
		count_start();
		sei();
	
		while(no < distance[dis] && !SPI_ReceiveFlag()){
			/*�f�o�b�O�p*/
			/*uart_putdec(no);
			uart_putstr("\n\r");
			if(noflag){
			uart_putdec(cot);
			uart_putchar(' ');
			uart_putdec(timr);
			uart_putchar(' ');
			uart_putdec((uint16_t)timbest);
			uart_putchar(' ');
			uart_putdec(OCR0A);
			uart_putchar(' ');
			uart_putstr("\n\r");
			noflag=0;
			}*/
		}
		//uart_putstr("finish");
		brake();
		uart_putdec(count);
		uart_putstr(",");
		uart_putdec(no/count);
		uart_putstr(".");
		SPI_SetSendData(1);
		return no;
	}
	
}
int mflag=0;
void count_start(void){
	count++;
	mflag=0;
	TCNT1 = 0; //���Z�b�g
	TIFR1=0x04;
	TIMSK1=0x01;
	TCCR1B=0x0b0000011; //start
}
void count_stop(void){
	TCCR1B=0x00; //�X�g�b�v
	//TIMSK1=0x00;//���荞�݋֎~
}


ISR(TIMER1_OVF_vect){
	OCR0A=OCR0A*best/(no-cot);
	TCNT1=0;
	cot=no;
	mflag=1;
	count++;
	//uart_putstr("over");
}

ISR(INT0_vect){
	no++;
	/*if((no-cot)>=ro){ //�������l�̉�]���ɒB�������ǂ���
		//timr = TCNT1 - timr;
		timr=TCNT1;
		OCR0A = (float)OCR0A*(timbest/timr);
		if(OCR0A<=20){
			OCR0A=30;
		}
		if(OCR0A>=0xFF){
			OCR0A=0xFF;
		}
		cot=no;
		TCNT1=0;
	}*/
	noflag=1;
}

#endif /* MOTOR_H_ */