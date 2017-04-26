
#include "uart.h"
//#include <iostm8.h>
#include "config.h"
#include "stm8s.h"

/* uart globals */

u8 uart_txd_in_ptr, uart_txd_out_ptr;
u8 uart_rxd_in_ptr, uart_rxd_out_ptr;
u8 UART_NL[] = {0x0d,0x0a,0};
u8 uart_txd_buffer[UART_TX_BUF_SIZE];
u8 uart_rxd_buffer[UART_RX_BUF_SIZE];


void uart_init(void)
/* initialize uart */
{
    UART2_CR1 = 0; // for Mbit  - 8bit
    UART2_CR3 = 0; // 1 stop bits;
    #define UART2_VALUE (unsigned short)((F_CLK*2/BAUD+1)/2)
    UART2_BRR2 = ((unsigned char)(UART2_VALUE>>8)) & 0xf0 | ((unsigned char)UART2_VALUE & 0x0f);
    UART2_BRR1 = (unsigned char)(UART2_VALUE>>4);
    UART2_CR2 = (0<<7)|(0<<6)|(1<<5)|(0<<4)|(1<<3)|(1<<2)|(0<<1)|(0<<0);//TIEN TCIEN RIEN ILIEN TEN REN RWU SBK
    // enable receiver, transmitter and interrupt from receiver
    uart_txd_in_ptr  = uart_txd_out_ptr = uart_rxd_in_ptr  = uart_rxd_out_ptr = 0;
}


#pragma vector = UART2_T_TXE_vector
__interrupt void UART2_T_TXE(void)
/* signal handler for uart data buffer empty interrupt */
{
  if ( uart_txd_in_ptr != uart_txd_out_ptr ){
    UART2_DR = uart_txd_buffer[uart_txd_out_ptr];
    uart_txd_out_ptr = (uart_txd_out_ptr + 1) & TX_MASK;
  }else{
    UART2_CR2_TIEN = 0;      // disable TIEN int 
  }
}


#pragma vector = UART2_R_RXNE_vector
__interrupt void UART2_R_RXNE(void)
/* signal handler for receive complete interrupt */
{
  UART2_SR_RXNE = 0;//In UART2 and UART3, it can also be cleared by writing 0
  uart_rxd_buffer[uart_rxd_in_ptr] = UART2_DR;
  uart_rxd_in_ptr = (uart_rxd_in_ptr + 1) & RX_MASK;
  if( uart_rxd_in_ptr == uart_rxd_out_ptr ) {
    uart_rxd_out_ptr =(uart_rxd_out_ptr + 1) & RX_MASK;
  }
}


s16 uart_getchar(void)
{
    u8 c;
    UART2_CR2_RIEN = 0;// disable RIEN int //disableInterrupts();
    if( uart_rxd_in_ptr != uart_rxd_out_ptr ) {
        c = uart_rxd_buffer[uart_rxd_out_ptr];              /* get character from buffer */
        uart_rxd_out_ptr = (uart_rxd_out_ptr + 1) & RX_MASK;
        UART2_CR2_RIEN = 1;// enable RIEN int //enableInterrupts();
        return c;
    }
    else{
        UART2_CR2_RIEN = 1;// enable RIEN int //enableInterrupts();
        return -1;                          /* buffer is empty */
    }
}


bool uart_putchar(u8 c)
{
  unsigned char next = (uart_txd_in_ptr + 1) & TX_MASK;
  UART2_CR2_TIEN = 0;      // disable TIEN int 
  if( next != uart_txd_out_ptr ) {/* buffer is not full */
        uart_txd_buffer[uart_txd_in_ptr] = c;               /* put character into buffer */
        uart_txd_in_ptr = next;
        UART2_CR2_TIEN = 1;   // enable TIEN int
        return TRUE;
  }else{
    UART2_CR2_TIEN = 1;   // enable TIEN int //enableInterrupts();
    return FALSE;/* buffer is full */
  }
}

/*
bool uart_putstr(u8 s[])
{
    u8 *c = s;

    while (*c)
        if (uart_putchar(*c))
            c++;

    return TRUE;
}

void uart_nl(void) // Send a 'new line' 
{
    uart_putstr(UART_NL);
}

bool uart_getstr(u8* bufer, u8 num)
{
	s16 c;

    while (num--)
	 {
	 	while ((c=uart_getchar()) == buffer_is_empty);
		if  (((u8)c == 0x0d) || ((u8)c == 0x0a))
		 {
			 *bufer=0;
			return TRUE;
		 }
		*bufer++=(u8)c;
	 }
	*bufer=0;
	return FALSE;
}

u8 mStrtoHex(u8 *s)
{
	u8 n;
	n=0;
	while(*s != 0)
	 {

	    if( *s >= '0' && *s <= '9')
		 {
			n = (n * 16) + (*s - '0');
			s++;
		 }
		else if(*s >= 'A' && *s <= 'F')
		 {
	 		n = (n * 16) + (*s - '7');
	 		s++;
		 }
		else if(*s >= 'a' && *s <= 'f')
		 {
	 		n = (n * 16) + (*s - 'W');
	 		s++;
		 }
		else
	 	s++;
	 }
	return n;
}

void mHextoStr(u8 num, u8 *s)
{
	u8 i,num1;
	*s++ = '0', *s++ = 'x';
	for(i=2;i>0;i--)
	 {
		num1=(num>>(4*(i-1))) & 0xF;
		if (num1<0xA) *s++ = 0x30 + num1;
			else *s++ = 0x37 + num1;
	 }
	*s = 0;
}
*/