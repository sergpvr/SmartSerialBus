
#ifndef UART_H
#define UART_H

#include "stm8s_type.h"
#include "F_CLK.h"
#define BAUD 9600
#define ESC            0x1b
#define UART_RX_BUF_SIZE    16   /* UART receive buffer size (must be 2^n, 2-256)  */
#define UART_TX_BUF_SIZE    16   /* UART transmit buffer size (must be 2^n, 2-256) */
#define buffer_is_empty   -1   
#define	RX_MASK		(UART_RX_BUF_SIZE-1)
#define	TX_MASK		(UART_TX_BUF_SIZE-1)
//===============================
/* prototypes */
extern void uart_init(void);
extern void uart_nl(void);
extern bool uart_putchar(u8 c);
extern s16  uart_getchar(void);
/*
extern bool uart_putstr(u8 s[]);
extern bool uart_getstr(u8* bufer, u8 num);
extern u8 mStrtoHex(u8 *s);
extern void mHextoStr(u8 num, u8 *s);
*/
#endif // UART_H
