/*
 * transmission.h
 *
 * Created: 2017/08/22 17:46:20
 *  Author: shun2
 */ 


#ifndef TRANSMISSION_H_
#define TRANSMISSION_H_

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>


#define rx_bufsize (25)
#define tx_bufsize (25)

#define UBRR_Value(b)       (20000000UL-8*(unsigned long)b)/(16*(unsigned long)b)
#define UBRR_U2XValue(b)    (20000000UL-4*(unsigned long)b)/( 8*(unsigned long)b)-1


void init_spi(void)
{
	DDRB = (1<<PORTB4); /* MISO出力、他は入力に設定 */
	SPCR = (1<<SPE); /* SPI許可設定 */
}
char SPI_SlaveReceive(void)
{
	while(!(SPSR & (1<<SPIF))); /* 受信(転送)完了まで待機 */
	return SPDR; /* 受信ﾃﾞｰﾀと共に復帰 */
}


void uart_putchar(char c);
void uart_putstr(const char *s);
void uart_putstr_p(const char *pgm_s);
uint8_t uart_getch(void);
uint8_t uart_rcv_size(void);
void init_uart(unsigned int baud);//whileの前に使用
void uart_puthex(uint8_t byte);
void uart_putdec(uint16_t data);     //数値


typedef struct
{
	char buf[tx_bufsize];  /*送信バッファサイズ*/
	uint8_t buf_in;     /* バッファに文字を押し込むときの位置*/
	uint8_t buf_size;   /*バッファに貯められている文字数 */
} TxBuf_t;
typedef struct
{
	char buf[rx_bufsize];
	uint8_t buf_in;     /* バッファに文字を押し込むときの位置*/
	uint8_t buf_size;   /*バッファに貯められている文字数 */
} RxBuf_t;

volatile TxBuf_t TxBuf;
volatile RxBuf_t RxBuf;

void uart_putchar(char c)
{
    while (TxBuf.buf_size > (tx_bufsize-1)) ;  /* バッファが空くまで待つ */
    cli(); //バッファを書き換える（読み出しも含む)操作は必ず割り込み禁止で!
    TxBuf.buf[TxBuf.buf_in++] = c;
    TxBuf.buf_size++;
    if (TxBuf.buf_in==tx_bufsize) TxBuf.buf_in=0;
    UCSR0B = UCSR0B | _BV(UDRIE0); /* UDR空き割り込みを有効にする。 */
    sei();
}

void uart_putstr(const char *s)
{
	char c;
	for(;;)
	{
		c = *s++;
		if (c==0) break;
		uart_putchar(c);
	}
}

void uart_putstr_p(const char *pgm_s)
{
	char c;
	c=pgm_read_byte(pgm_s);
	while ((c=pgm_read_byte(pgm_s++)) != 0)
	uart_putchar(c);
}


void uart_puthex(uint8_t byte)
{
	uint8_t i;
	uart_putchar('0');
	uart_putchar('x');
	i=(byte>>4);
	if(i<10)
	uart_putchar(i+'0');
	else
	uart_putchar(i+'a'-10);

	i=(byte&0x0f);
	if(i<10)
	uart_putchar(i+'0');
	else
	uart_putchar(i+'a'-10);

}
void uart_puthex2(uint16_t word)
{
	uint8_t i;
	uart_putchar('0');
	uart_putchar('x');
	i=((word>>12)&0x0f);
	if(i<10)
	uart_putchar(i+'0');
	else
	uart_putchar(i+'a'-10);
	i=((word>>8)&0x0f);
	if(i<10)
	uart_putchar(i+'0');
	else
	uart_putchar(i+'a'-10);
	i=((word>>4)&0x0f);
	if(i<10)
	uart_putchar(i+'0');
	else
	uart_putchar(i+'a'-10);
	i=((word)&0x0f);
	if(i<10)
	uart_putchar(i+'0');
	else
	uart_putchar(i+'a'-10);
}

void uart_putdec(uint16_t data){
	//uart_putchar(data/10000000 + '0');
	//data %= 10000000;
	//uart_putchar(data/1000000 + '0');
	//data %= 1000000;
	//uart_putchar(data/100000 + '0');
	//data %= 100000;
	uart_putchar(data/10000 + '0');
	data %= 10000;
	uart_putchar(data/1000 + '0');
	data %= 1000;
	uart_putchar(data/100 + '0');
	data %= 100;
	uart_putchar(data/10 + '0');
	data %= 10;
	uart_putchar(data + '0');
}
void uart_putdec2(int data){
	uart_putchar(data/1000000000000 + '0');
	data %= 1000000000000;
	uart_putchar(data/100000000000 + '0');
	data %= 100000000000;
	uart_putchar(data/10000000000 + '0');
	data %= 10000000000;
	uart_putchar(data/1000000000 + '0');
	data %= 1000000000;
	uart_putchar(data/100000000 + '0');
	data %= 100000000;
	uart_putchar(data/10000000 + '0');
	data %= 10000000;
	uart_putchar(data/1000000 + '0');
	data %= 1000000;
	uart_putchar(data/100000 + '0');
	data %= 100000;
	uart_putchar(data/10000 + '0');
	data %= 10000;
	uart_putchar(data/1000 + '0');
	data %= 1000;
	uart_putchar(data/100 + '0');
	data %= 100;
	uart_putchar(data/10 + '0');
	data %= 10;
	uart_putchar(data + '0');
}

uint8_t uart_getch(void)
{
	char c;
	uint8_t buf_out;
	while (!RxBuf.buf_size) ;
	cli();
	buf_out=RxBuf.buf_in-RxBuf.buf_size;
	if (RxBuf.buf_in<RxBuf.buf_size) {
		buf_out += rx_bufsize;
	}
	c = RxBuf.buf[buf_out];
	if (RxBuf.buf_size==0) {
		RxBuf.buf_size += rx_bufsize;
	}
	RxBuf.buf_size--;
	sei();
	return c;
}

uint8_t uart_rcv_size(void){
	return RxBuf.buf_size;
}

void init_uart(unsigned int baud){
	UCSR0A = 0b00000000;
	UCSR0B = _BV(TXEN0)|_BV(RXCIE0)|_BV(UDRIE0)|_BV(RXEN0);
	UBRR0H=UBRR_Value(baud) >> 8;
	UBRR0L=UBRR_Value(baud) &  0xFF;
	RxBuf.buf_in = RxBuf.buf_size = 0;
	TxBuf.buf_size = 0;    /*バッファを空にする */
}





ISR(USART_UDRE_vect){
	/* TxBuf.buf_size==0の状態でDATA割り込みを許可すると変なことになります。注意。 */
	uint8_t buf_out;
	buf_out=TxBuf.buf_in-TxBuf.buf_size;
	if (TxBuf.buf_in<TxBuf.buf_size) buf_out += tx_bufsize;
	UDR0 = TxBuf.buf[buf_out];
	TxBuf.buf_size--;
	if (TxBuf.buf_size==0) UCSR0B = UCSR0B & ~_BV(UDRIE0); /* UDR空き割り込み禁止 */
}

ISR(USART_RX_vect){
	uint8_t c;
	c = UDR0;
	if (RxBuf.buf_size>=rx_bufsize) {
		return;
		//フロー制御なし
	}
	RxBuf.buf[RxBuf.buf_in++]=c;
	if (RxBuf.buf_in>=rx_bufsize) RxBuf.buf_in=0;
	RxBuf.buf_size++;
}





#endif /* TRANSMISSION_H_ */