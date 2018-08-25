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
PORTB |= _BV(PB1); 　　//PB1ピンだけをHighにし、残りは変化させない
PORTB &= ~_BV(PB3); 　　//PB3ピンだけをLowにし、残りは変化させない


0.0000512←1024分周の一カウント = 0.0512ms
TCNT1の上限値=65535
最大カウント秒数 3.355392s
20RPM想定 1ガタガタ = 0.0003571428571s
10ガタガタ=0.003571428571約70カウント(1024分周)

************************************************/
//////////////////////////Define達/////////////////////////////
#define CS (1<<PORTC2)
#define INB (1<<PORTC3)
#define INA (1<<PORTC4)
#define EN (1<<PORTC5)

#include "rx_spi.hpp"

#define rotation 8400
///////////////////////////////////////////グローバル宣言////////////////////////////////////////////////////
void init_motor(void); //initiarize
void advance(void); //前進
void reverce(void); //後進
void brake(void); //ブレーキ
void move(int i); //動く(1:前進,2:後進　それ以外:ブレーキ)
int motor(void);//SPI読み→モーター動かす
void count_start(void);
void count_stop(void);
void motor_3byte_test(void);
int motor_distance(void);

//////////////////////////////////////////グローバル変数////////////////////////////////////////////////////
int best = 0;
uint16_t timr = 0;
uint8_t speed = 0;//スピード変数
int cot = 0; //一時回転数用変数
unsigned int no=0;
int ro = 10; //タイマー比較　回転数しきい値
float timbest = 3900; //タイマー比較　ベスト値
float timcnt = 3900;
uint8_t noflag=0;
//long int distance[8]={1500,rotation/3,rotation/4,0,rotation};
//uint16_t distance[7] = {0,47,94,29,0,0,0} ; //1マス,2マス,超新地 --上昇端、下降端
uint16_t distance[7] = {0,95,186,60,35,0,0};//1マス,2マス,超新地 --両端割り込み
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
unsigned int mflag=0;

int motor(void){
	no=0;
	cot=0;
	timr=0;
	while(!SPI_ReceiveFlag());
	SPI_SetSendData(2);
	
	uint8_t num = SPI_ReceiveData();
	uint8_t fb = num>>6;
	speed = (num>>3)&0b00000111;
	uint8_t dis = num&0b00000111;
	uart_putdec(fb);
	uart_putdec(speed);
	uart_putdec(dis);
	if(speed==1){
		best=10;
	}
	else{
		best = speed*4+2;
	}
	uart_putdec(best);
	uart_putstr("\n\r");
	//timbest = timcnt*5/speed;
	
	if(fb==3 || speed==0){
		brake();
		OCR0A = 0;
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
	//OCR0A = (speed*10) - 8;
	if(speed==1){
		OCR0A =60;
	}
	else{	
		OCR0A = speed * 33;
	}
		
	//count_start();
	sei();
	
	while(no < distance[dis] && !SPI_ReceiveFlag()){
		/*デバッグ用*/
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
	SPI_SetSendData(1);
	/*uart_putstr("finish-Over:");
	uart_putdec(mflag);
	uart_putstr("OCR0A:");
	uart_putdec(OCR0A);
	uart_putstr("\n\r");*/
	brake();
	
	return no;
}
void count_start(void){
	mflag=0; //リセット
	TCNT1 = 0;
	TIFR1=0x04;
	TIMSK1=0x01;
	TCCR1B=0b0000100; //start
	TCNT1 = 0;
}
void count_stop(void){
	TCCR1B=0x00; //ストップ
	//TIMSK1=0x00;//割り込み禁止
}


ISR(TIMER1_OVF_vect){
	OCR0A=OCR0A*best/(no-cot);
	TCNT1=0;
	cot=no;
	mflag++;
	//uart_putstr("over");
}

ISR(INT0_vect){
	no++;
	/*if((no-cot)>=ro){ //しきい値の回転数に達したかどうか
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
	//noflag=1;
}

#endif /* MOTOR_H_ */