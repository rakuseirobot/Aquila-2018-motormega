/*
 * motorv5.h
 *
 * Created: 2018/03/18 16:59:09
 *  Author: Shun
 */ 
/*
This Source for motor made by Pololu moving with PI control.
Rising Edge!

And Overflow Interrupt will happen when Timer = 65535;
*/

#ifndef MOTORV5_H_
#define MOTORV5_H_

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
#define tinit 0
///////////////////////////////////////////グローバル宣言////////////////////////////////////////////////////
void init_motor(void); //initiarize
void advance(void); //前進
void reverce(void); //後進
void brake(void); //ブレーキ
void move(uint8_t i); //動く(1:前進,2:後進　それ以外:ブレーキ)
void motor(void);//SPI読み→モーター動かす
void count_start(void);
void count_stop(void);
void motor_3byte_test(void);
int motor_distance(void);



////////////////////////////////////////////////////////////////////////////////////////
/*
配列　1つめ:通常時　2つめ:修正動作時
*/
double p_gain[2] = {1.0,1.0,};//1.01	;//1.05;//P制御(比例)ゲイン//要調整
double s_gain[2] = {0.5,0.5,};//0.1;//I制御(積分)ゲイン//要調整
double p=0,s=0;
/////////////////////////////////////////////////////////////////////////////////////////

uint8_t debugno = 0;
//////////////////////////////////////////グローバル変数////////////////////////////////////////////////////
uint16_t timr = 0;
uint8_t speed = 0;//スピード変数
int16_t cot = 0; //一時回転数用変数
int16_t no=0;
unsigned int dtemp;
int ro = 10; //タイマー比較　回転数しきい値
float timbest = 3900; //タイマー比較　ベスト値
float timcnt = 3900;
uint8_t noflag=0;
double ocr = 0;//OCR0A値
int32_t kasan = 0;//I制御用　誤差
int32_t best = 0;//オーバーフロー時の最適カウント数
int32_t nox=0;
uint8_t fb = 3;//0:未定義,1:前進,2:後退,3:停止
uint16_t ircount=0;
//long int distance[8]={1500,rotation/3,rotation/4,0,rotation};
//uint16_t distance[7] = {0,47,94,29,0,0,0} ; //1マス,2マス,超新地 --上昇端、下降端
//uint16_t distance[7] = {0,95,186,60,35,0,0};//1マス,2マス,超新地,Half --両端割り込み
uint16_t distance[7] = {0,2900,6200,1790,1000,0,0};//Max65535
void init_motor(void){
	DDRC |= CS|INB|INA|EN;
	DDRD |= (0<<PIND2)|(1<<PORTD6);
	//PORTD |= (1<<PORTD2);
	EICRA = 0b00000010;
	/*
	00 : LOW level Interrupt
	01 : Both Edge Interrupt
	10 : Falling Edge Interrupt
	11 : Rising Edge Interrupt
	*/
	EIFR  = 0b00000001;
	EIMSK = 0b00000001;
	TCNT1 = tinit;
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
void motor(void){
	ircount = no = kasan =  0;
	uint8_t num = 0;
	while(!SPI_ReceiveFlag()){
		//uart_putstr("Waiting...\n\r");
	}
	SPI_SetSendData(2);
	num = SPI_ReceiveData();
	uint8_t fb = num>>6;
	speed = (num>>3)&0b00000111;
	uint8_t dis = num&0b00000111;
	if(debugno){
	uart_putstr("fb:");
	uart_putdec(fb);
	uart_putstr("sp:");
	uart_putdec(speed);
	uart_putstr("dis:");
	uart_putdec(dis);
	}
	if (speed == 1){
		p = p_gain[1];
		s = s_gain[1];
		speed = 1;
		best = 80;
	}
	else{
		best = speed*65;  //要調整 OCR0A = 255 : 140 //??
	}
	//uart_putstr("best:");
	//uart_putdec(best);
	//uart_putstr("\n\r");
	//timbest = timcnt*5/speed;
	
	if(fb==3 || speed==0){
		brake();
		SPI_SetSendData(1);
		temp= 1;
		return;
	}
	else if(fb==1){
		advance();
	}
	else if(fb==2){
		reverce();
	}
	
	
	
	if (1==((num>>3)&0b00000111)){
		OCR0A = 50;
	}
	else{
		OCR0A = speed * 35;
		count_start();
		sei();
	}
	noflag = 1;
	while(ircount*best-kasan < distance[dis] && !SPI_ReceiveFlag()){
		/*デバッグ用*/
		//uart_putdec(no);
		//uart_putstr("\n\r");
		if(noflag&&debugno){
			uart_putstr("nox:");
			uart_putdec(nox);\
			uart_putstr("kasan:");
			uart_putdec(kasan);
			uart_putstr("OCR0A:");
			uart_putdec(OCR0A);
			uart_putstr("ocr:");
			uart_putdec(ocr);
			uart_putstr	("\n\r");
			noflag=0;
		}
	}
	//count_stop();
	/*
	uart_putstr("finish-Over:");
	uart_putdec(mflag);
	uart_putstr("OCR0A:");
	uart_putdec(OCR0A);*/
	brake();
	SPI_SetSendData(1);
	if(debugno){
		uart_putstr(" Finished!");
		uart_putstr("\n\r");
	}
	return;
}
void count_start(void){
	mflag=0; //リセット
	TCNT1 = tinit;
	TIFR1=0x01; //Enable to Overflow Flag.
	TIMSK1=0x01; //Enable to Overflow Interrupt.
	TCCR1B=0x03; //start Last 3bit : 分周
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
	cli();
	TIMSK1=0x00;
	TIFR1=0x00;
	TCCR1B=0x00; //ストップ
	TCNT1 = tinit;
	ircount = no = kasan = 0;
	//TIMSK1=0x00;//割り込み禁止
}
void isr_prog(void){
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
	return;
}

ISR(TIMER1_OVF_vect){
	TCNT1=tinit;// +19999 カウント幅　45536
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
		ocr=1;
	}
	OCR0A=(uint8_t)ocr;
	no=0;
	ircount++;
	noflag = 1;
	count_start();
	//isr_prog();
}

ISR(INT0_vect){
	no++;
	EIFR  = 0b00000001;
	EIMSK = 0b00000001;
}


#endif /* MOTORV5_H_ */