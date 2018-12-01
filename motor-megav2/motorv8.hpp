/*
 * motorv8.h
 *
 * Created: 2018/08/26 17:54:02
 *  Author: shun2
 */ 
/*
Write on 8/26/2018
*/

#ifndef MOTORV8_H_
#define MOTORV8_H_

/*
This Source for motor made by Pololu moving with PI control.
Rising Edge!

And Overflow Interrupt will happen when Timer = 65535;
*/
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
#define SIG (1<<PORTD3)

#include "rx_spi.hpp"
#include "avr/sfr_defs.h"

#define rotation 6400
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
//////////////////////////////////////////////////////////////////////////////////////
/*
1. I,D制御ゲイン→0
2. P制御ゲイン調整　(修正時速度、通常時速度) (波がなくなる狭間でなるべく小さく)
3. I制御ゲイン調整　(kasanが波打つぐらいで小さく)
4. D制御ゲイン調整　(回転時に速度が波打たないように)(修正時の速度でも)
*/
////////////////////////////////////////////////////////////////////////////////////////


/*double p = 0.18;//1.01	;//1.05;//P制御(比例)ゲイン//要調整
double s = 0.01;//0.1;//I制御(積分)ゲイン//要調整
double d = 0.15;*/

/*
double p = 0.08;
double s = 0.015;
double d = 0.2;*/


double p = 5;
double s = 0;
double d = 0;

/////////////////////////////////////////////////////////////////////////////////////////

uint8_t debugno = 0;


//////////////////////////////////////////グローバル変数////////////////////////////////////////////////////
uint16_t timr = 0;
uint8_t speed = 0;//スピード変数
int32_t cot = 0; //一時回転数用変数
int32_t no=0;
unsigned int dtemp;
int ro = 10; //タイマー比較　回転数しきい値
float timbest = 3900; //タイマー比較　ベスト値
float timcnt = 3900;
uint8_t noflag=0;
double ocr = 0;//OCR0A値
int32_t kasan = 0;//I制御用　誤差
int32_t best = 0;//オーバーフロー時の最適カウント数
int32_t nox=0;
int32_t dev[2] = {0,0};
uint8_t fb = 3;//0:未定義,1:前進,2:後退,3:停止
int32_t ircount=0;	
/*
//long int distance[8]={1500,rotation/3,rotation/4,0,rotation};
//uint16_t distance[7] = {0,47,94,29,0,0,0} ; //1マス,2マス,超新地 --上昇端、下降端
//uint16_t distance[7] = {0,95,186,60,35,0,0};//1マス,2マス,超新地,Half --両端割り込み
*/


uint16_t distance[8] = {0,2388,4670,1600,940,2300,0};   //Max65535		//empty,1block,2blocks,Turn,Half,Compass
//uint16_t distance[8] = {0,5318,4670,1500,940,2300,0};   //Max65535		//empty,1block,2blocks,Turn,Half,Compass

void init_motor(void){
	DDRC |= CS|INB|INA|EN;
	DDRD |= (0<<PIND2)|(0<<PIND3)|(1<<PORTD6);
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

bool fixflag=false;

void motor(void){
	fixflag=false;
	ircount = no = kasan = dev[0] = dev[1] = 0;
	uint8_t num = 0;
	uart_putstr("\n\rs:");
	uart_putdec(s*100);
	uart_putstr("\n\r");
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
	if(speed==1){
		speed = 3;
	}
	best = speed*8;  //要調整 OCR0A = 255 : 140 //??
	uart_putstr("best:");
	uart_putdec(best);
	uart_putstr("\n\r");
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
	
	OCR0A = ocr = speed * 30;
	count_start();
	sei();

	noflag = 1;
	while(((ircount*best)-kasan)+no < distance[dis] && !SPI_ReceiveFlag()){
		if(!bit_is_clear(PIND,PIND3)){
			PORTC = INB|INA|EN;
			cli();
			TIMSK1=0x00;
			TIFR1=0x00;
			TCCR1B=0x00; //ストップ
			uart_putstr("stop\n\r");
			while(!bit_is_clear(PIND,PIND3));
			TIFR1=0x01; //Enable to Overflow Flag.
			TIMSK1=0x01; //Enable to Overflow Interrupt.
			TCCR1B=0x02; //start Last 3bit : 分周
			sei();
			if(fb==1){
				advance();
			}
			else if(fb==2){
				reverce();
			}
		}
		if(fixflag){
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
			dev[1] = best - no;
			ocr+=p*(best-no)+s*kasan-d*(dev[1]-dev[0]);
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
			dev[0] = dev[1];
			count_start();
			//isr_prog();
			fixflag=false;
			uart_putdec(best-nox);
			uart_putstr(",");
			uart_putstr("\n\r");
		}
		/*デバッグ用*/
		/*uart_putdec(no);
		uart_putstr("\n\r");
		if(noflag&&debugno){
			uart_putstr("nox:");
			uart_putdec(nox);
			uart_putstr("kasan:");
			uart_putdec(kasan);
			uart_putstr("OCR0A:");	
			uart_putdec(OCR0A);
			uart_putstr("count:");
			uart_putdec(((ircount*best)-kasan)+no);
			uart_putstr	("\n\r");
			noflag=0;
		}*/
	}
	//count_stop();
	
	uart_putstr("finish-Over:");
	uart_putdec(mflag);
	uart_putstr("OCR0A:");
	uart_putdec(OCR0A);
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
	TCCR1B=0x02; //start Last 3bit : 分周
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

ISR(TIMER1_OVF_vect){
	fixflag=true;
}

ISR(INT0_vect){
	no++;
	EIFR  = 0b00000001;
	EIMSK = 0b00000001;
}







#endif /* MOTORV8_H_ */