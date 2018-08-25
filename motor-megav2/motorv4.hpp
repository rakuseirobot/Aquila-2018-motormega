/*
 * motorv4.h
 *
 * Created: 2018/03/16 23:22:19
 *  Author: robot
 */ 
/*
This Source for motor made by Pololu moving with PI control.
Both Edge!
*/

#ifndef MOTORV4_H_
#define MOTORV4_H_

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
#define tinit -20000
///////////////////////////////////////////�O���[�o���錾////////////////////////////////////////////////////
void init_motor(void); //initiarize
void advance(void); //�O�i
void reverce(void); //��i
void brake(void); //�u���[�L
void move(uint8_t i); //����(1:�O�i,2:��i�@����ȊO:�u���[�L)
int motor(void);//SPI�ǂ݁����[�^�[������
void count_start(void);
void count_stop(void);
void motor_3byte_test(void);
int motor_distance(void);

//////////////////////////////////////////�O���[�o���ϐ�////////////////////////////////////////////////////
uint16_t timr = 0;
uint8_t speed = 0;//�X�s�[�h�ϐ�
unsigned cot = 0; //�ꎞ��]���p�ϐ�
int no=0;
unsigned int dtemp;
int ro = 10; //�^�C�}�[��r�@��]���������l
float timbest = 3900; //�^�C�}�[��r�@�x�X�g�l
float timcnt = 3900;
uint8_t noflag=0;
double p = 1.05;//1.05;//P����(���)�Q�C��//�v����
double s = 0.05;//0.1;//I����(�ϕ�)�Q�C��//�v����
double ocr = 0;//OCR0A�l
int kasan = 0;//I����p�@�덷
int best = 0;//�I�[�o�[�t���[���̍œK�J�E���g��
int nox=0;
uint8_t fb = 3;//0:����`,1:�O�i,2:���,3:��~
int ircount=0;
//long int distance[8]={1500,rotation/3,rotation/4,0,rotation};
//uint16_t distance[7] = {0,47,94,29,0,0,0} ; //1�}�X,2�}�X,���V�n --�㏸�[�A���~�[
//uint16_t distance[7] = {0,95,186,60,35,0,0};//1�}�X,2�}�X,���V�n,Half --���[���荞��
uint16_t distance[7] = {0,5950,12100,3800,2000,0,0};//Max65535
void init_motor(void){
	DDRC |= CS|INB|INA|EN;
	DDRD |= (0<<PIND2)|(1<<PORTD6);
	//PORTD |= (1<<PORTD2);
	EICRA = 0b00000001;
	/*
	00 : LOW level Interrupt
	01 : Both Edge Interrupt
	10 : Falling Edge Interrupt
	11 : Rising Edge Interrupt
	*/
	EIFR  = 0b00000001;
	EIMSK = 0b00000001;
	TCCR0A = 0b10000011;
	TCCR0B = 0b00000011;
	TCNT1 = tinit;
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
void move(uint8_t i){
	if(i==1){
		advance();
	}
	else if(i==2){
		reverce();
	}
	else brake();
}
int temp = 0;
unsigned int mflag=0;
/*
1:30
5:80
*/
int motor(void){
	no=0;
	cot=0;
	timr=0;
	uint8_t num = 0;
	while(!SPI_ReceiveFlag());
	num = SPI_ReceiveData();
	SPI_SetSendData(2);
	uint8_t fb = num>>6;
	speed = (num>>3)&0b00000111;
	uint8_t dis = num&0b00000111;
	uart_putstr("fb:");
	uart_putdec(fb);
	uart_putstr("sp:");
	uart_putdec(speed);
	uart_putstr("dis:");
	uart_putdec(dis);
	if (speed == 1){
		speed = 5;
	}
	best = speed*30;  //�v���� OCR0A = 255 :285
	uart_putstr("best:");
	uart_putdec(best);
	uart_putstr("\n\r");
	//timbest = timcnt*5/speed;
	
	if(fb==3 || speed==0){
		brake();
		SPI_SetSendData(1);
		temp= 1;
		return no;
	}
	else if(fb==1){
		advance();
	}
	else if(fb==2){
		reverce();
	}
	
	OCR0A = speed * 30;
	ocr = speed * 8;
	
		
	count_start();
	sei();
	
	noflag = 1;
	while(ircount*best-kasan < distance[dis] && !SPI_ReceiveFlag()){
		/*�f�o�b�O�p*/
		//uart_putdec(no);
		//uart_putstr("\n\r");
		if(noflag){
			uart_putstr("nox:");
			uart_putdec(nox);\
			uart_putstr("kasan:");
			uart_putdec(kasan);
			uart_putstr("OCR0A:");
			uart_putdec(OCR0A);
			uart_putstr("\n\r");
			noflag=0;
		}
	}
	count_stop();
	SPI_SetSendData(1);
	uart_putstr("finish-Over:");
	uart_putdec(mflag);
	uart_putstr("OCR0A:");
	uart_putdec(OCR0A);
	uart_putstr("\n\r");
	brake();
	
	return no;
}
void count_start(void){
	mflag=0; //���Z�b�g
	//TCNT1 = tinit;
	TIFR1=0x01; //Enable to Overflow Flag.
	TIMSK1=0x01; //Enable to Overflow Interrupt.
	TCCR1B=0x03; //start Last 3bit : ����
	/* 65535
	000:Stop
	001:clk/0 = 0.000005ms/count : 0.327675ms
	010:clk/8 = 0.0004ms/count : 26.214ms
	011:clk/64 = 0.0032ms/count : 209.712ms
	100:clk/256 = 0.0128ms/count : 838.848ms
	101:clk/1024 = 0.0512ms/count : 3355.392ms
	110,111:External Clock
	*/
	noflag = 1;
}
void count_stop(void){
	TIMSK1 &= ~0x01;
	TCCR1B=0x00; //�X�g�b�v
	TCNT1 = tinit;
	ircount = no = kasan = 0;
	//TIMSK1=0x00;//���荞�݋֎~
}


ISR(TIMER1_OVF_vect){
	TCNT1=-20000;// +19999 �J�E���g���@45536
	/*
	clk/0 : 0.22768ms
	clk/8 : 18.2144ms
	clk/64 : 145.7152ms
	clk/256 : 582.8608ms
	clk/1024 : 2331.4432ms
	*/
	nox=no;//for uart
	kasan += best - no;
	ocr+=p*(best-no)+s*kasan;
	//uart_putdec(ocr);
	if(ocr>=255){
		ocr=255;
	}
	else if(ocr<=0){
		ocr=0;
	}
	OCR0A=(uint8_t)ocr;
	no=0;
	ircount++;
	noflag = 1;
	count_start();
}

ISR(INT0_vect){
	no++;
	EIFR  = 0b00000001;
	EIMSK = 0b00000001;
}







#endif /* MOTORV4_H_ */