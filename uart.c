/*
 * Author: gofftang
 * Created on 2016/7/4, 21:00
 */


#include <xc.h>
#include "system.h"

void uart_init(long baud_rate)
{
#ifdef CONFIG_UART
    TRISB5 = 1; //rx
    TRISB7 = 1; //tx

    SPBRG = ((SYS_FREQ / 16) / baud_rate) - 1;

    BRGH = 1;   //Fast baudrate
    SYNC = 0;   //Asynchonous
    SPEN = 1;   //Enable serial pins
    CREN = 1;   //Enable reception
    SREN = 0;   //Not effect
    TXIE = 0;   //Disable tx interrupts
    RCIE = 1;   //Enable tx interrupts
    TX9 = 0;    //8-bit transmission
    RX9 = 0;    //8-bit reception
    TXEN = 0;   //Reset transmitter
    TXEN = 1;   //Enable transmitter
    
    PEIE = 1;
    GIE = 1;
#endif
}

#define get_hex_ascii(hex) (((hex) < 10) ? ((hex) + '0') : (((hex) - 10) + 'A'))

#ifdef CONFIG_UART
#define uart_wait_tx() while (!TXIF);  //Wait for previous transmission to finish

#if (UART_FUNC & UART_FUNC_RX)
#define uart_wait_rx() while (!RCIF);  //Wait for transmission to receive
#endif
#endif

#if (UART_FUNC & UART_FUNC_RX)
unsigned char serial_getc(void) {
    if (OERR) {
        CREN = 0;
        CREN = 1;
    }

    uart_wait_rx();

    return RCREG;
}
#endif

void serial_putc(unsigned char ch)
{
#ifdef CONFIG_UART
    if (ch == '\n') {
        uart_wait_tx();
        TXREG = '\r';
    }

    uart_wait_tx();
    TXREG = ch;
#endif
}

void serial_puts(const char *s)
{
#ifdef CONFIG_UART
    while (*s) {
		serial_putc(*s++);
	}
#endif
}

void serial_put_hex(unsigned int hex)
{
#ifdef CONFIG_UART
    char ch[3];
    ch[0] = get_hex_ascii((hex >> 4) & 0xF);
    ch[1] = get_hex_ascii(hex & 0xF);
    ch[2] = 0;
	serial_puts(ch);
#endif
}
